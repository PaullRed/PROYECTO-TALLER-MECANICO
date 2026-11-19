#include "Mantenimiento.h"
#include "Estructura.h"
#include "Persistencia.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <direct.h>

using namespace std;

int verificarHeaderArchivo(const string& ruta, size_t tamanoRegistro, const string& nombreArchivo, int& problemas) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) {
        cout << "ROTO: No se pudo abrir " << nombreArchivo << "\n";
        problemas++;
        return 0;
    }

    ifstream archivo(ruta, ios::binary | ios::ate);
    if (!archivo.is_open()) {
        problemas++;
        return 0;
    }

    long tamanoTotal = static_cast<long>(archivo.tellg());
    long bytesRegistros = tamanoTotal - static_cast<long>(sizeof(ArchivoHeader));
    int registrosReales = (bytesRegistros > 0) ? static_cast<int>(bytesRegistros / static_cast<long>(tamanoRegistro)) : 0;

    if (header.cantidadRegistros != registrosReales) {
        cout << "ROTO: Header de " << nombreArchivo << " indica " << header.cantidadRegistros
             << " registros; el archivo contiene " << registrosReales << "\n";
        problemas++;
    }

    int activosReales = 0;
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);

    for (int i = 0; i < registrosReales; i++) {
        if (tamanoRegistro == sizeof(Vehiculo)) {
            Vehiculo v;
            archivo.read(reinterpret_cast<char*>(&v), sizeof(Vehiculo));
            if (!v.eliminado) activosReales++;
        } else if (tamanoRegistro == sizeof(Mecanico)) {
            Mecanico m;
            archivo.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));
            if (!m.eliminado) activosReales++;
        } else if (tamanoRegistro == sizeof(Repuesto)) {
            Repuesto r;
            archivo.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));
            if (!r.eliminado) activosReales++;
            if (r.stock < 0) {
                cout << "ROTO: Repuesto ID " << r.id << " (" << r.codigo << ") tiene stock negativo: " << r.stock << "\n";
                problemas++;
            }
        } else if (tamanoRegistro == sizeof(OrdenServicio)) {
            OrdenServicio o;
            archivo.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
            if (!o.eliminado) activosReales++;
        }
    }

    archivo.close();

    if (header.registrosActivos != activosReales) {
        cout << "ROTO: Header de " << nombreArchivo << " indica " << header.registrosActivos
             << " activos; el archivo contiene " << activosReales << "\n";
        problemas++;
    }

    return registrosReales;
}

void verificarIntegridad(const string& rutaVehiculos,
                         const string& rutaMecanicos,
                         const string& rutaRepuestos,
                         const string& rutaOrdenes) {
    int problemas = 0;

    int totalVehiculos = verificarHeaderArchivo(rutaVehiculos, sizeof(Vehiculo), "vehiculos.bin", problemas);
    int totalMecanicos = verificarHeaderArchivo(rutaMecanicos, sizeof(Mecanico), "mecanicos.bin", problemas);
    int totalRepuestos = verificarHeaderArchivo(rutaRepuestos, sizeof(Repuesto), "repuestos.bin", problemas);
    int totalOrdenes = verificarHeaderArchivo(rutaOrdenes, sizeof(OrdenServicio), "ordenes.bin", problemas);

    ifstream archivoOrdenes(rutaOrdenes, ios::binary);
    if (archivoOrdenes.is_open()) {
        archivoOrdenes.seekg(sizeof(ArchivoHeader), ios::beg);
        OrdenServicio o;
        for (int i = 0; i < totalOrdenes; i++) {
            archivoOrdenes.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));

            Vehiculo v;
            if (!obtenerVehiculoPorID(rutaVehiculos, o.idVehiculo, v)) {
                cout << "ROTO: Orden #" << o.id << " apunta al vehiculo ID " << o.idVehiculo << " (no existe en vehiculos.bin)\n";
                problemas++;
            } else if (strcmp(o.estado, "ABIERTA") == 0 && v.eliminado) {
                cout << "ROTO: Orden ABIERTA #" << o.id << " apunta al vehiculo ID " << o.idVehiculo << " que esta eliminado\n";
                problemas++;
            }

            Mecanico m;
            if (!obtenerMecanicoPorID(rutaMecanicos, o.idMecanico, m)) {
                cout << "ROTO: Orden #" << o.id << " apunta al mecanico ID " << o.idMecanico << " (no existe en mecanicos.bin)\n";
                problemas++;
            } else if (strcmp(o.estado, "ABIERTA") == 0 && m.eliminado) {
                cout << "ROTO: Orden ABIERTA #" << o.id << " apunta al mecanico ID " << o.idMecanico << " que esta dado de baja\n";
                problemas++;
            }

            for (int j = 0; j < o.numDetalles; j++) {
                Repuesto r;
                if (!obtenerRepuestoPorID(rutaRepuestos, o.detalles[j].idRepuesto, r)) {
                    cout << "ROTO: Orden #" << o.id << " linea " << (j + 1) << " apunta al repuesto ID "
                         << o.detalles[j].idRepuesto << " (no existe en repuestos.bin)\n";
                    problemas++;
                }
            }
        }
        archivoOrdenes.close();
    }

    cout << "\nREPORTE DE INTEGRIDAD DEL SISTEMA\n";
    cout << "Vehiculos verificados : " << totalVehiculos << "\n";
    cout << "Mecanicos verificados : " << totalMecanicos << "\n";
    cout << "Repuestos verificados: " << totalRepuestos << "\n";
    cout << "Ordenes verificadas   : " << totalOrdenes << "\n";
    cout << "Problemas encontrados : " << problemas << "\n";
    cout << "Estado: " << (problemas == 0 ? "SISTEMA INTEGRO" : "SISTEMA CON ERRORES") << "\n";
}

bool copiarArchivoBinario(const string& origen, const string& destino) {
    ifstream in(origen, ios::binary);
    if (!in.is_open()) return false;

    ofstream out(destino, ios::binary);
    if (!out.is_open()) {
        in.close();
        return false;
    }

    char buffer[4096];
    while (in.read(buffer, sizeof(buffer))) {
        out.write(buffer, in.gcount());
    }
    if (in.gcount() > 0) {
        out.write(buffer, in.gcount());
    }

    in.close();
    out.close();
    return true;
}

bool crearBackup(const string& rutaTaller,
                 const string& rutaVehiculos,
                 const string& rutaMecanicos,
                 const string& rutaRepuestos,
                 const string& rutaOrdenes,
                 string& rutaCarpetaCreada) {
    time_t ahora = time(nullptr);
    tm* tiempoLocal = localtime(&ahora);
    char nombreCarpeta[64];
    strftime(nombreCarpeta, sizeof(nombreCarpeta), "backup_%Y-%m-%d_%H-%M", tiempoLocal);

    _mkdir(nombreCarpeta);
    rutaCarpetaCreada = string(nombreCarpeta);

    bool ok1 = copiarArchivoBinario(rutaTaller, rutaCarpetaCreada + "/taller.bin");
    bool ok2 = copiarArchivoBinario(rutaVehiculos, rutaCarpetaCreada + "/vehiculos.bin");
    bool ok3 = copiarArchivoBinario(rutaMecanicos, rutaCarpetaCreada + "/mecanicos.bin");
    bool ok4 = copiarArchivoBinario(rutaRepuestos, rutaCarpetaCreada + "/repuestos.bin");
    bool ok5 = copiarArchivoBinario(rutaOrdenes, rutaCarpetaCreada + "/ordenes.bin");

    return ok1 && ok2 && ok3 && ok4 && ok5;
}