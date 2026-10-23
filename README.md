# Sistema de Gestion para Taller Mecanico en C++

Sistema modular de gestion para taller mecanico desarrollado en C++ bajo el paradigma procedimental y modular. El proyecto administra los registros de vehiculos, mecanicos, inventario de repuestos y ordenes de servicio, implementado estrictamente sin el uso de la libreria estandar de plantillas (STL) para demostrar el manejo manual de estructuras de datos y gestion dinamica de memoria.

## Caracteristicas Principales

* **Gestion de Vehiculos:** Registro, busqueda (por ID, placa o propietario), actualizacion de datos y eliminacion de registros.
* **Gestion de Mecanicos:** Registro de personal tecnico con asignacion de especialidad y control de limite de ordenes abiertas (maximo 3 asignaciones concurrentes).
* **Inventario de Repuestos:** Control de stock, alertas de inventario minimo, actualizacion de precios y registro de entradas de mercancia.
* **Ordenes de Servicio:**
  * Apertura de ordenes vinculando vehiculos y mecanicos asignados.
  * Adicion y retiro dinamico de repuestos con ajuste automatico del inventario.
  * Calculo automatizado de costos (subtotal de repuestos + mano de obra) y cierre formal de ordenes.
  * Consulta de historiales de servicio por vehiculo y filtrado por estado de la orden (ABIERTA, CERRADA, ANULADA).
* **Control de Robustez:** Validacion de flujos de entrada en consola e integracion para la ejecucion de pruebas automatizadas mediante redireccion de archivos de texto (compatible con EOF).

## Estructura del Proyecto

```
PROYECTO/
├── pruebas/
│   ├── entrada.txt            # Secuencia de entradas automatizadas para pruebas de integracion
│   └── salida.txt             # Resultado y traza generada por la ejecucion automatizada
├── Estructura.h               # Definicion de las estructuras de datos principales
├── Sistema.h / .cpp           # Logica central e inicializacion del sistema
├── Vehiculo.h / .cpp          # Modulo para la gestion de vehiculos
├── Mecanico.h / .cpp          # Modulo para la gestion de mecanicos
├── Repuesto.h / .cpp          # Modulo para el control de inventario
├── OrdenServicio.h / .cpp     # Modulo para el flujo de ordenes y calculos financieros
├── main.cpp                   # Punto de entrada y gestion de menups de usuario
└── README.md                  # Documentacion formal del proyecto
```

## Requisitos de Software

* Compilador de C++ compatible con el estandar C++11 o superior (ejemplo: `g++` / GCC sobre entornos MinGW para Windows, Linux o macOS).
* Entorno de Desarrollo Integrado (IDE): Visual Studio Code (recomendado) o terminal de comandos equivalente.

## Compilacion y Ejecucion

### 1. Compilacion desde Consola de Comandos (`g++`)

Para compilar la totalidad del proyecto modular, ejecute el siguiente comando en la raiz del directorio:

```bash
g++ main.cpp Sistema.cpp Vehiculo.cpp Mecanico.cpp Repuesto.cpp OrdenServicio.cpp -o taller.exe
```

### 2. Ejecucion Interactiva

Una vez finalizado el proceso de compilacion sin errores, ejecute el programa resultante:

```bash
.\taller.exe
```

## Ejecucion de Pruebas Automatizadas

El sistema soporta la evaluacion de casos de uso mediante redireccion de flujos de entrada de texto plano, facilitando las pruebas de integracion sin intervencion manual.

### Pruebas en PowerShell

Para inyectar el conjunto de datos definido en `pruebas/entrada.txt`:

```powershell
Get-Content pruebas/entrada.txt | .\taller.exe
```

### Generacion de Archivos de Salida (Evidencia)

Para canalizar la salida del programa y generar el reporte correspondiente en `pruebas/salida.txt`:

```powershell
Get-Content pruebas/entrada.txt | .\taller.exe > pruebas/salida.txt
```

## Autor

* **Paul Rojas** - [*PaullRed*](https://github.com/PaullRed)