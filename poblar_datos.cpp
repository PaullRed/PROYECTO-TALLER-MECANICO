#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <ctime>
#include "Estructura.h"
#include "Persistencia.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include "OrdenServicio.h"

using namespace std;

namespace {
const string RUTA_TALLER    = "datos/taller.bin";
const string RUTA_VEHICULOS = "datos/vehiculos.bin";
const string RUTA_MECANICOS = "datos/mecanicos.bin";
const string RUTA_REPUESTOS = "datos/repuestos.bin";
const string RUTA_ORDENES   = "datos/ordenes.bin";

void copiarCadena(char* destino, const char* origen, size_t tamano) {
    strncpy(destino, origen, tamano);
    destino[tamano - 1] = '\0';
}

bool prepararArchivosLimpios() {
    ofstream fTaller(RUTA_TALLER, ios::binary | ios::trunc);
    if (!fTaller.is_open()) {
        return false;
    }
    Taller t{};
    copiarCadena(t.nombre, "Autoservicio El Lago", sizeof(t.nombre));
    copiarCadena(t.rif, "J-12345678-9", sizeof(t.rif));
    copiarCadena(t.direccion, "Av. 5 de Julio, Maracaibo", sizeof(t.direccion));
    copiarCadena(t.telefono, "0412-1234567", sizeof(t.telefono));
    t.fechaCreacion = std::time(nullptr);
    t.fechaUltimaModificacion = t.fechaCreacion;
    fTaller.write(reinterpret_cast<const char*>(&t), sizeof(Taller));
    bool okTaller = fTaller.good();
    fTaller.close();
    if (!okTaller) return false;

    const string archivosEntidad[] = {RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES};
    for (const string& ruta : archivosEntidad) {
        ofstream fReset(ruta, ios::binary | ios::trunc);
        if (!fReset.is_open()) return false;
        ArchivoHeader h{};
        h.cantidadRegistros = 0;
        h.proximoID = 1;
        h.registrosActivos = 0;
        h.version = 1;
        fReset.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
        bool ok = fReset.good();
        fReset.close();
        if (!ok) return false;
    }
    return true;
}

Vehiculo crearVehiculo(const char* placa, const char* marca, const char* modelo, int anio, int km,
                     const char* prop, const char* cedula, const char* tel) {
    Vehiculo v{};
    copiarCadena(v.placa, placa, sizeof(v.placa));
    copiarCadena(v.marca, marca, sizeof(v.marca));
    copiarCadena(v.modelo, modelo, sizeof(v.modelo));
    v.anio = anio;
    v.kilometraje = km;
    copiarCadena(v.nombrePropietario, prop, sizeof(v.nombrePropietario));
    copiarCadena(v.cedulaPropietario, cedula, sizeof(v.cedulaPropietario));
    copiarCadena(v.telefonoPropietario, tel, sizeof(v.telefonoPropietario));
    return v;
}

Mecanico crearMecanico(const char* nombre, const char* cedula, const char* esp, const char* fechaIng) {
    Mecanico m{};
    copiarCadena(m.nombre, nombre, sizeof(m.nombre));
    copiarCadena(m.cedula, cedula, sizeof(m.cedula));
    copiarCadena(m.especialidad, esp, sizeof(m.especialidad));
    copiarCadena(m.fechaIngreso, fechaIng, sizeof(m.fechaIngreso));
    return m;
}

Repuesto crearRepuesto(const char* codigo, const char* nombre, const char* marca, float precio, int stock, int stockMin) {
    Repuesto r{};
    copiarCadena(r.codigo, codigo, sizeof(r.codigo));
    copiarCadena(r.nombre, nombre, sizeof(r.nombre));
    copiarCadena(r.marca, marca, sizeof(r.marca));
    r.precio = precio;
    r.stock = stock;
    r.stockMinimo = stockMin;
    return r;
}
} // namespace

int main() {
    if (!prepararArchivosLimpios()) {
        cerr << "Error preparando archivos en datos/.\n";
        return 1;
    }

    int idTmp = 0;

    // 1. Registrar 8 vehiculos (IDs 1..8)
    Vehiculo vehiculos[] = {
        crearVehiculo("AB123CD", "Toyota", "Corolla", 2018, 65000, "Carlos Mendez", "V-18452100", "0414-5550101"),
        crearVehiculo("EF456GH", "Chevrolet", "Aveo", 2015, 92000, "Maria Rondón", "V-20114890", "0424-5550102"),
        crearVehiculo("IJ789KL", "Ford", "Fiesta", 2019, 48000, "Jose Chirinos", "V-16987321", "0412-5550103"),
        crearVehiculo("MN012OP", "Hyundai", "Tucson", 2021, 31000, "Laura Fuenmayor", "V-22345987", "0416-5550104"),
        crearVehiculo("QR345ST", "Nissan", "Sentra", 2017, 78000, "Carlos Urdaneta", "V-19002341", "0414-5550105"),
        crearVehiculo("UV678WX", "Kia", "Rio", 2020, 54000, "Elena Briceño", "V-24876123", "0424-5550106"),
        crearVehiculo("YZ901AB", "Jeep", "Cherokee", 2016, 110000, "Andres Villalobos", "V-15678432", "0412-5550107"),
        crearVehiculo("CD234EF", "Mazda", "3", 2022, 22500, "Sofia Morales", "V-26123789", "0416-5550108")
    };
    for (int i = 0; i < 8; i++) {
        if (!registrarVehiculo(RUTA_VEHICULOS, vehiculos[i], idTmp)) {
            cerr << "Fallo registrando vehiculo " << (i + 1) << "\n";
            return 1;
        }
    }

    // 2. Registrar 4 mecanicos (IDs 1..4) y dar de baja al #4 despues de cerrar sus ordenes
    Mecanico mecanicos[] = {
        crearMecanico("Jose Perez", "V-14223344", "MOTOR", "2022-01-15"),
        crearMecanico("Ana Villalobos", "V-19887766", "FRENOS", "2023-03-10"),
        crearMecanico("Luis Fuenmayor", "V-21554433", "ELECTRICIDAD", "2023-08-01"),
        crearMecanico("Pedro Socorro", "V-17665544", "GENERAL", "2021-11-20")
    };
    for (int i = 0; i < 4; i++) {
        if (!registrarMecanico(RUTA_MECANICOS, mecanicos[i], idTmp)) {
            cerr << "Fallo registrando mecanico " << (i + 1) << "\n";
            return 1;
        }
    }

    // 3. Registrar 12 repuestos (IDs 1..12) con stock suficiente y algunos cerca del minimo
    Repuesto repuestos[] = {
        crearRepuesto("REP-001", "Filtro de aceite", "Toyota", 14.50f, 35, 8),
        crearRepuesto("REP-002", "Pastillas de freno delanteras", "Brembo", 38.00f, 24, 6),
        crearRepuesto("REP-003", "Bujias iridio (juego)", "NGK", 28.00f, 30, 6),
        crearRepuesto("REP-004", "Bateria 12V 800A", "Duncan", 95.00f, 15, 4),
        crearRepuesto("REP-005", "Correa de distribucion", "Gates", 45.00f, 18, 5),
        crearRepuesto("REP-006", "Disco de freno ventilado", "TRW", 52.00f, 16, 4),
        crearRepuesto("REP-007", "Amortiguador delantero", "KYB", 68.00f, 14, 4),
        crearRepuesto("REP-008", "Bomba de agua", "Aisin", 58.00f, 12, 3),
        crearRepuesto("REP-009", "Sensor de oxigeno", "Bosch", 42.00f, 20, 5),
        crearRepuesto("REP-010", "Aceite sintetico 5W-30 (4L)", "Mobil", 36.00f, 28, 6),
        crearRepuesto("REP-011", "Filtro de aire motor", "Mann", 16.00f, 25, 6),
        crearRepuesto("REP-012", "Liquido de frenos DOT4", "Castrol", 11.50f, 30, 6)
    };
    for (int i = 0; i < 12; i++) {
        if (!registrarRepuesto(RUTA_REPUESTOS, repuestos[i], idTmp)) {
            cerr << "Fallo registrando repuesto " << (i + 1) << "\n";
            return 1;
        }
    }

    // 4. Crear 10 ordenes (IDs 1..10):
    //    - Ordenes #1, #2, #3, #4: CERRADAS (3 de ellas con >= 4 lineas de detalle)
    //    - Ordenes #5, #6, #7: ABIERTAS
    //    - Ordenes #8, #9, #10: ANULADAS
    int ord = 0;

    // Orden #1 (CERRADA - 5 lineas) - Vehiculo 1, Mecanico 1
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 1, 1, 65100, "Afinacion mayor y revision", "2026-10-01", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 1, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 3, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 10, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 11, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 9, 1);
    cerrarOrdenServicio(RUTA_ORDENES, ord, 65.0f, "2026-10-02");

    // Orden #2 (CERRADA - 4 lineas) - Vehiculo 2, Mecanico 2
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 2, 2, 92150, "Sistema de frenos completo", "2026-10-03", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 2, 2);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 6, 2);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 12, 2);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 1, 1);
    cerrarOrdenServicio(RUTA_ORDENES, ord, 55.0f, "2026-10-04");

    // Orden #3 (CERRADA - 4 lineas) - Vehiculo 3, Mecanico 3
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 3, 3, 48200, "Mantenimiento preventivo y correa", "2026-10-05", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 5, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 8, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 10, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 11, 1);
    cerrarOrdenServicio(RUTA_ORDENES, ord, 70.0f, "2026-10-06");

    // Orden #4 (CERRADA - 2 lineas) - Vehiculo 4, Mecanico 4 (luego se da de baja al mecanico 4)
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 4, 4, 31120, "Cambio de bateria y revision general", "2026-10-07", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 4, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 1, 1);
    cerrarOrdenServicio(RUTA_ORDENES, ord, 30.0f, "2026-10-07");

    // Dar de baja al Mecanico 4 (ya no tiene ordenes abiertas; conserva historial en orden #4)
    darDeBajaMecanicoLogico(RUTA_MECANICOS, 4, RUTA_ORDENES);

    // Orden #5 (ABIERTA - 2 lineas) - Vehiculo 5, Mecanico 1
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 5, 1, 78100, "Golpeteo en suspension delantera", "2026-11-10", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 7, 2);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 12, 1);

    // Orden #6 (ABIERTA - 1 linea) - Vehiculo 6, Mecanico 2
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 6, 2, 54100, "Ajuste de pastillas delanteras", "2026-11-11", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 2, 1);

    // Orden #7 (ABIERTA - 2 lineas) - Vehiculo 7, Mecanico 3
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 7, 3, 110200, "Falla de sensor y encendido", "2026-11-12", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 9, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 3, 1);

    // Orden #8 (ANULADA - 2 lineas, stock devuelto al anular) - Vehiculo 8, Mecanico 1
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 8, 1, 22600, "Revision inicial cancelada por cliente", "2026-11-02", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 1, 1);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 11, 1);
    anularOrdenServicio(RUTA_ORDENES, RUTA_REPUESTOS, ord);

    // Orden #9 (CERRADA luego ANULADA para probar anulacion de orden cerrada) - Vehiculo 1, Mecanico 2
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 1, 2, 65300, "Cambio de discos de freno", "2026-11-03", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 6, 2);
    cerrarOrdenServicio(RUTA_ORDENES, ord, 40.0f, "2026-11-03");
    anularOrdenServicio(RUTA_ORDENES, RUTA_REPUESTOS, ord);

    // Orden #10 (ABIERTA - 1 linea) - Vehiculo 2, Mecanico 3
    abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, 2, 3, 92300, "Revision electrica de luces", "2026-11-13", ord);
    agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, ord, 4, 1);

    cout << "Datos pre-poblados generados correctamente en datos/.\n";
    cout << "Resumen: 8 vehiculos, 4 mecanicos (1 de baja), 12 repuestos, 10 ordenes (3 abiertas, 4 cerradas, 3 anuladas; 3 cerradas con >=4 lineas).\n";
    return 0;
}