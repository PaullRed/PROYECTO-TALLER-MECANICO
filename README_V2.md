# Sistema de Gestión de Taller Mecánico: "Autoservicio El Lago" (Fase 2)

Evolución del sistema de gestión de taller mecánico implementando **persistencia en archivos binarios** (`<fstream>`) y **acceso aleatorio directo** (`seekg` / `seekp`), eliminando el almacenamiento volátil en memoria RAM y garantizando la integridad referencial entre múltiples entidades.

---

## 1. Instrucciones de Compilación y Ejecución

### Requisitos
* Compilador **G++** (MinGW-w64 o GCC) con soporte para **C++17** o superior.
* Carpeta `datos/` creada en la raíz del proyecto.

### Compilación del Sistema Principal
Desde la terminal en la carpeta raíz del proyecto, ejecutar:
```bash
g++ -std=c++17 -Wall -Wextra -mconsole main.cpp Persistencia.cpp Vehiculo.cpp Mecanico.cpp Repuesto.cpp OrdenServicio.cpp Reportes.cpp Mantenimiento.cpp -o taller.exe
```

### Ejecución
```bash
./taller.exe
```

### Poblado Inicial de Datos (Opcional / Reinicio de Base de Datos)
Para regenerar los archivos binarios con el conjunto de datos de prueba obligatorio (8 vehículos, 4 mecánicos, 12 repuestos y 10 órdenes):
```bash
g++ -std=c++17 -Wall -Wextra -mconsole poblar_datos.cpp Persistencia.cpp Vehiculo.cpp Mecanico.cpp Repuesto.cpp OrdenServicio.cpp -o poblar_datos.exe
./poblar_datos.exe
```

---

## 2. Tamaños de Estructuras (`sizeof`) y Fórmula de Acceso Aleatorio

### Tabla de Tamaños en Disco (Arquitectura x64 - MinGW)
*(Nota: Valores calculados considerando alineación estándar de memoria de 8 bytes para `time_t` y 4 bytes para `int`/`float`)*.

| Estructura | Archivo Asociado | Tamaño (`sizeof`) | Observaciones |
| :--- | :--- | :---: | :--- |
| `ArchivoHeader` | Todos (excepto `taller.bin`) | **16 bytes** | 4 enteros (`int` de 4 bytes cada uno) |
| `Taller` | `datos/taller.bin` | **360 bytes** | Registro único sin header |
| `Vehiculo` | `datos/vehiculos.bin` | **288 bytes** | Incluye datos del vehículo, propietario y auditoría |
| `Mecanico` | `datos/mecanicos.bin` | **176 bytes** | Incluye estado `eliminado` para baja lógica |
| `Repuesto` | `datos/repuestos.bin` | **208 bytes** | Incluye precios, stocks y auditoría |
| `DetalleOrden` | Embebido en `OrdenServicio` | **12 bytes** | `idRepuesto` (4B) + `cantidad` (4B) + `precioUnitario` (4B) |
| `OrdenServicio` | `datos/ordenes.bin` | **448 bytes** | Incluye arreglo fijo `detalles[15]` (180 bytes) |

### Fórmula de Posición en Bytes
Para acceder directamente a cualquier registro cuyo índice físico base-cero sea $i$ ($i \in \{0, 1, 2, \dots\}$), se utiliza la función `calcularPosicionRegistro(indiceFisico, tamanoEstructura)`:

$$\text{posición} = \text{sizeof}(\text{ArchivoHeader}) + (i \times \text{sizeof}(\text{Estructura}))$$

---

## 3. Relación entre ID e Índice Físico

* **Deducción:** Dado que el sistema asigna los IDs de forma estrictamente correlativa empezando en $1$ (`proximoID = 1`) en orden de inserción, y que **nunca se realizan eliminaciones físicas ni reordenamientos** de registros dentro del archivo, existe una correspondencia biunívoca directa entre el `id` lógico y el índice físico $i$:
  $$i = \text{id} - 1$$
  Esto permite ubicar cualquier entidad por su ID en complejidad temporal $O(1)$ con una sola operación `seekg()`, sin recorrer el archivo secuencialmente.

* **¿Qué cambio invalidaría esta relación?**
  1. **Compactación o borrado físico:** Si se eliminara físicamente un registro intermedio desplazando los registros posteriores hacia arriba para llenar el hueco, los IDs posteriores dejarían de coincidir con `id - 1`.
  2. **Ordenamiento en disco:** Si se reordenaran los registros dentro del archivo binario por otro criterio (ej. alfabéticamente por placa o nombre).
  3. **Asignación manual de IDs no secuenciales:** Si se permitiera al usuario ingresar IDs arbitrarios (ej. saltar del ID 5 al ID 20).

---

## 4. Diseño de Persistencia del Detalle de la Orden

### Estructura Utilizada
Se optó por un **arreglo estático embebido de tamaño fijo** dentro de `OrdenServicio`, acompañado de un contador entero (`numDetalles`):

```cpp
struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario; // Precio histórico al momento de agregar
}; // sizeof = 12 bytes

struct OrdenServicio {
    // ... campos administrativos ...
    static const int MAX_DETALLES = 15;
    DetalleOrden detalles[MAX_DETALLES]; // 15 * 12 = 180 bytes
    int numDetalles;                     // 4 bytes
    // ... marcas de tiempo ...
};
```

### Espacio Ocupado en Disco y Desperdicio
* **Tamaño fijo por orden:** Cada `OrdenServicio` ocupa exactamente **448 bytes** en `ordenes.bin`, independientemente de cuántas líneas tenga.
* **Orden con 1 línea:** Ocupa **448 bytes** en disco. De los 180 bytes reservados para el arreglo `detalles`, se utilizan 12 bytes y quedan **168 bytes sin usar** (desperdicio interno).
* **Orden con 15 líneas:** Ocupa los mismos **448 bytes** en disco. Se utilizan los 180 bytes completos del arreglo (**0 bytes desperdiciados**).

### Alternativa Considerada y Descartada
* **Alternativa:** Crear un archivo independiente `datos/detalles.bin` con su propio `ArchivoHeader`, donde cada registro representara una línea vinculada mediante un campo `idOrden`.
* **Por qué se descartó:** Aunque elimina el desperdicio de espacio para órdenes con pocos repuestos, rompe el acceso aleatorio $O(1)$ al consultar una orden: para reconstruir el detalle de una sola orden habría que recorrer secuencialmente todo `detalles.bin` (o implementar listas enlazadas en disco mediante offsets), multiplicando las operaciones de entrada/salida (I/O) y complicando la consistencia transaccional al quitar líneas o anular órdenes. Al ser el máximo de 15 líneas acotado y conocido, el arreglo embebido garantiza que leer una orden completa requiera una única lectura de disco.

### Resolución de la Operación "Quitar una Línea"
1. Se ubica la orden en disco mediante acceso directo (`seekg`) y se verifica que su estado sea `"ABIERTA"`.
2. Se busca el `idRepuesto` dentro del subarreglo activo `detalles[0 ... numDetalles - 1]`.
3. Se guarda la `cantidad` de esa línea para devolverla al inventario.
4. Para mantener la contigüidad del arreglo sin dejar huecos intermedios, se **desplazan hacia la izquierda** todos los elementos posteriores a la posición eliminada (`detalles[i] = detalles[i + 1]`) y se decrementa `numDetalles--`.
5. Se suma la cantidad devuelta al `stock` del repuesto en `repuestos.bin` y se sobrescribe la orden modificada en su misma posición (`seekp`).

---

## 5. Arquitectura y Firmas de Funciones por Capa

Todas las funciones de persistencia y lógica devuelven `bool` para indicar éxito (`true`) o fallo (`false`), evitando retornar punteros a estructuras en memoria. Los datos recuperados se entregan por referencia (`Entidad&`), y las búsquedas múltiples llenan un arreglo estático acotado por `maxResultados` con una bandera `bool& hayMas`.

### Capa de Persistencia (`Persistencia.h`)
* `bool inicializarArchivoEntidad(const std::string& rutaArchivo);` — Crea el archivo con `ArchivoHeader` inicial (`version = 1`, `proximoID = 1`) si no existe.
* `bool leerHeader(const std::string& rutaArchivo, ArchivoHeader& header);` — Lee los primeros 16 bytes del archivo.
* `bool actualizarHeader(const std::string& rutaArchivo, const ArchivoHeader& header);` — Sobrescribe el header en el offset `0`.
* `long calcularPosicionRegistro(int indiceFisico, size_t tamanoRegistro);` — Calcula el desplazamiento exacto en bytes.

### Capa de Lógica — Vehículos (`Vehiculo.h`)
* `bool registrarVehiculo(const std::string& rutaArchivo, Vehiculo datosVehiculo, int& idAsignado);` — Valida año (1950–2027), kilometraje $\ge 0$ y placa única entre activos.
* `bool obtenerVehiculoPorID(const std::string& rutaArchivo, int id, Vehiculo& vehiculoEncontrado);` — Lectura directa $O(1)$. Retorna también registros con `eliminado == true` para que la presentación distinga entre "eliminado" e "inexistente".
* `bool obtenerVehiculoPorPlaca(const std::string& rutaArchivo, const std::string& placa, Vehiculo& vehiculoEncontrado);` — Búsqueda secuencial registro a registro solo entre activos.
* `bool buscarVehiculosPorPropietarioParcial(const std::string& rutaArchivo, const std::string& fragmentoNombre, Vehiculo resultados[], int maxResultados, int& cantidadEncontrados, bool& hayMas);` — Coincidencia parcial *case-insensitive* acotada.
* `bool actualizarVehiculo(const std::string& rutaArchivo, int id, const Vehiculo& nuevosDatos);` — Modifica marca, modelo, año y datos del propietario sin alterar placa ni kilometraje.
* `bool eliminarVehiculoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes);` — Verifica en `ordenes.bin` que el vehículo no tenga órdenes asociadas antes de marcar `eliminado = true`.

### Capa de Lógica — Mecánicos (`Mecanico.h`)
* `bool registrarMecanico(const std::string& rutaArchivo, Mecanico datosMecanico, int& idAsignado);` — Valida cédula única y especialidad permitida.
* `bool obtenerMecanicoPorID(const std::string& rutaArchivo, int id, Mecanico& mecanicoEncontrado);` — Acceso directo $O(1)$; entrega activos y dados de baja para preservar nombres en el historial de órdenes.
* `bool darDeBajaMecanicoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes);` — Rechaza la baja si existen órdenes en estado `"ABIERTA"` asignadas al mecánico.

### Capa de Lógica — Repuestos (`Repuesto.h`)
* `bool registrarRepuesto(const std::string& rutaArchivo, Repuesto datosRepuesto, int& idAsignado);` — Valida código único entre activos, precio $> 0$ y stocks $\ge 0$.
* `bool obtenerRepuestoPorID(const std::string& rutaArchivo, int id, Repuesto& repuestoEncontrado);` — Acceso directo $O(1)$.
* `bool obtenerRepuestoPorCodigo(const std::string& rutaArchivo, const std::string& codigo, Repuesto& repuestoEncontrado);` — Búsqueda por código exacto entre activos.
* `bool buscarRepuestosPorNombreParcial(const std::string& rutaArchivo, const std::string& fragmentoNombre, Repuesto resultados[], int maxResultados, int& cantidadEncontrados, bool& hayMas);` — Coincidencia parcial *case-insensitive*.
* `bool registrarEntradaMercancia(const std::string& rutaArchivo, int idRepuesto, int cantidad);` — Incrementa stock si `cantidad > 0`.
* `bool cambiarPrecioRepuesto(const std::string& rutaArchivo, int idRepuesto, float nuevoPrecio);` — Actualiza precio vigente sin alterar el `precioUnitario` histórico guardado en órdenes previas.
* `bool eliminarRepuestoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes);` — Verifica que el repuesto no figure en el detalle de ninguna orden antes de darlo de baja.

### Capa de Lógica — Órdenes de Servicio (`OrdenServicio.h`)
* `bool abrirOrdenServicio(...);` — Valida vehículo activo sin otra orden abierta, mecánico activo con $< 3$ órdenes abiertas y kilometraje $\ge$ actual; actualiza kilometraje del vehículo y crea la orden.
* `bool agregarRepuestoAOrden(...);` — Valida orden abierta, stock suficiente y límite de 15 líneas; acumula si el repuesto ya existe en la orden o crea línea nueva con precio histórico, descontando stock.
* `bool quitarRepuestoDeOrden(...);` — Retira la línea de la orden abierta y reintegra las unidades a `repuestos.bin`.
* `bool cerrarOrdenServicio(...);` — Valida orden abierta, mano de obra $\ge 0$ y al menos 1 línea o mano de obra $> 0$; cambia estado a `"CERRADA"`.
* `bool anularOrdenServicio(...);` — Permite anular órdenes `"ABIERTA"` o `"CERRADA"`, devolviendo todo el stock de sus líneas a `repuestos.bin`.
* `float calcularTotalOrdenServicio(const OrdenServicio& orden);` — Calcula $\text{ManoObra} + \sum (\text{cantidad} \times \text{precioUnitario})$.
* `bool obtenerOrdenPorID(const std::string& rutaOrdenes, int id, OrdenServicio& ordenEncontrada);` — Acceso directo $O(1)$.

---

## 6. Preguntas de Análisis

### 1. Experimento con punteros en disco (`DetalleOrden*` del Proyecto 1)
* **¿Qué se observó al intentar acceder a las líneas tras reiniciar el programa?**
  Al leer el registro desde el archivo binario en una nueva ejecución del programa y tratar de desreferenciar el puntero `DetalleOrden*`, el programa sufre un **fallo de segmentación (`Segmentation Fault`)** o muestra **valores basura/corruptos** en pantalla.
* **¿Por qué ocurre?**
  La función `write()` copia literalmente los bytes de la estructura en RAM hacia el archivo. En el caso de un puntero, lo que se guarda en disco no son los datos de los repuestos, sino la **dirección de memoria virtual** (por ejemplo, `0x000001a4f2b01040`) donde el sistema operativo alojó ese arreglo dinámico durante la primera ejecución. Cuando el programa se cierra, esa memoria se libera; al volver a ejecutarlo, el sistema operativo asigna direcciones de memoria completamente distintas, por lo que ese número guardado en disco apunta a una dirección no asignada al proceso o ocupada por otros datos.

### 2. Interrupción entre escrituras en operaciones compuestas (Detalle vs. Stock)
* **Inconsistencia según el orden de escritura:**
  Al agregar un repuesto a una orden se modifican dos archivos: `repuestos.bin` (descuento de stock) y `ordenes.bin` (inclusión de la línea). Si ocurre un fallo eléctrico o cierre abrupto justo entre ambas escrituras:
  * *Si se escribe primero el detalle en `ordenes.bin` y luego falla la escritura del stock en `repuestos.bin`:* La orden queda con el repuesto asignado y cobrado al cliente, pero el inventario físico nunca se descontó (el sistema cree tener más unidades disponibles de las que realmente hay, generando sobreventa).
  * *Si se escribe primero el descuento de stock en `repuestos.bin` y luego falla la escritura en `ordenes.bin`:* El inventario refleja una unidad menos, pero ninguna orden registra haberla consumido.
* **¿Qué orden deja el daño más fácil de detectar o corregir, y por qué?**
  Es preferible **actualizar primero el detalle en `ordenes.bin` y luego descontar el stock en `repuestos.bin`** (o viceversa si se audita inventario, pero desde el punto de vista de auditoría contable, tener la orden escrita primero permite reconstruir el consumo exacto). Si la orden ya quedó persistida con sus líneas en `ordenes.bin`, un proceso de auditoría puede recorrer todas las órdenes válidas (`ABIERTA` y `CERRADA`), sumar las cantidades consumidas de cada repuesto y recalcular el stock real esperado. En cambio, si se descuenta el stock primero y la orden nunca se guarda, no queda rastro alguno de qué orden o cliente provocó esa merma en el inventario.

### 3. Eliminación física vs. Baja lógica de mecánicos
* **¿Qué habría pasado con el historial si se hubiera permitido la eliminación física?**
  Si se eliminara físicamente el registro de un mecánico (o se reutilizara su espacio/ID), todas las órdenes históricas (`CERRADA` o `ANULADA`) que almacenan su `idMecanico` quedarían con una **referencia huérfana (integridad referencial rota)**. Al consultar una orden pasada o generar el reporte de facturación por mecánico, el sistema no encontraría el registro correspondiente o, peor aún, mostraría el nombre de un mecánico nuevo que hubiera tomado ese ID.
* **¿Por qué la baja lógica lo resuelve?**
  Al marcar únicamente `eliminado = true` sin borrar los bytes del archivo ni alterar su `id`, el registro permanece intacto en `mecanicos.bin`. De este modo, la capa de negocio impide asignarle nuevas órdenes activas (protegiendo la operación futura), mientras que las consultas históricas y los reportes financieros siguen resolviendo su `idMecanico` mediante acceso directo para mostrar su nombre y estadísticas acompañados de la etiqueta `[BAJA]`.
