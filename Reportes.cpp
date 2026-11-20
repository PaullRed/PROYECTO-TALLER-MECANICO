#include "Reportes.h"
#include "Estructura.h"
#include "Persistencia.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include "OrdenServicio.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>

using namespace std;

void reporteHistorialVehiculo(const string& placa,
                              const string& rutaVehiculos,
                              const string& rutaMecanicos,
                              const string& rutaOrdenes) {
    Vehiculo v;
    if (!obtenerVehiculoPorPlaca(rutaVehiculos, placa, v)) {
        cout << "No se encontro un vehiculo activo con la placa " << placa << ".\n";
        return;
    }

    cout << "\n========================================\n";
    cout << "      HISTORIAL DE VEHICULO\n";
    cout << "========================================\n";
    cout << "Placa      : " << v.placa << "\n";
    cout << "Vehiculo   : " << v.marca << " " << v.modelo << " (" << v.anio << ")\n";
    cout << "Kilometraje: " << v.kilometraje << " km\n";
    cout << "Propietario: " << v.nombrePropietario << " (" << v.cedulaPropietario << ")\n";
    cout << "----------------------------------------\n";
    cout << setw(8) << left << "Orden"
         << setw(12) << left << "Fecha"
         << setw(22) << left << "Mecanico"
         << setw(10) << left << "Estado"
         << setw(10) << right << "Total" << "\n";
    cout << "----------------------------------------\n";

    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) return;

    ifstream archivoOrdenes(rutaOrdenes, ios::binary);
    if (!archivoOrdenes.is_open()) return;

    archivoOrdenes.seekg(sizeof(ArchivoHeader), ios::beg);
    OrdenServicio o;
    float totalFacturadoCerradas = 0.0f;
    int conteoOrdenes = 0;

    for (int i = 0; i < headerOrdenes.cantidadRegistros; i++) {
        archivoOrdenes.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
        if (o.idVehiculo == v.id) {
            Mecanico m;
            string nombreMec = "Desconocido";
            if (obtenerMecanicoPorID(rutaMecanicos, o.idMecanico, m)) {
                nombreMec = m.nombre;
                if (m.eliminado) nombreMec += " (BAJA)";
            }

            float totalOrden = calcularTotalOrdenServicio(o);
            if (strcmp(o.estado, "CERRADA") == 0) {
                totalFacturadoCerradas += totalOrden;
            }

            cout << setw(8) << left << o.id
                 << setw(12) << left << o.fechaApertura
                 << setw(22) << left << nombreMec.substr(0, 21)
                 << setw(10) << left << o.estado
                 << setw(10) << right << fixed << setprecision(2) << totalOrden << "\n";
            conteoOrdenes++;
        }
    }
    archivoOrdenes.close();

    if (conteoOrdenes == 0) {
        cout << "Este vehiculo no posee ordenes registradas.\n";
    }
    cout << "----------------------------------------\n";
    cout << "Total facturado (Ordenes CERRADAS): $" << fixed << setprecision(2) << totalFacturadoCerradas << "\n";
    cout << "========================================\n";
}

void reporteRepuestosMasUtilizados(const string& rutaRepuestos,
                                   const string& rutaOrdenes) {
    ArchivoHeader headerRep;
    ArchivoHeader headerOrd;
    if (!leerHeader(rutaRepuestos, headerRep) || !leerHeader(rutaOrdenes, headerOrd)) return;

    const int TOP_N = 10;
    int topIds[TOP_N] = {0};
    int topUnidades[TOP_N] = {0};

    ifstream fRep(rutaRepuestos, ios::binary);
    if (!fRep.is_open()) return;

    for (int i = 0; i < headerRep.cantidadRegistros; i++) {
        fRep.seekg(calcularPosicionRegistro(i, sizeof(Repuesto)), ios::beg);
        Repuesto r;
        fRep.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));

        int unidadesConsumidas = 0;
        ifstream fOrd(rutaOrdenes, ios::binary);
        if (fOrd.is_open()) {
            fOrd.seekg(sizeof(ArchivoHeader), ios::beg);
            OrdenServicio o;
            for (int j = 0; j < headerOrd.cantidadRegistros; j++) {
                fOrd.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
                if (strcmp(o.estado, "CERRADA") == 0) {
                    for (int k = 0; k < o.numDetalles; k++) {
                        if (o.detalles[k].idRepuesto == r.id) {
                            unidadesConsumidas += o.detalles[k].cantidad;
                        }
                    }
                }
            }
            fOrd.close();
        }

        if (unidadesConsumidas > 0) {
            for (int pos = 0; pos < TOP_N; pos++) {
                if (unidadesConsumidas > topUnidades[pos]) {
                    for (int mover = TOP_N - 1; mover > pos; mover--) {
                        topUnidades[mover] = topUnidades[mover - 1];
                        topIds[mover] = topIds[mover - 1];
                    }
                    topUnidades[pos] = unidadesConsumidas;
                    topIds[pos] = r.id;
                    break;
                }
            }
        }
    }
    fRep.close();

    cout << "\n========================================\n";
    cout << "   TOP 10 REPUESTOS MAS UTILIZADOS\n";
    cout << "========================================\n";
    cout << setw(5) << left << "#"
         << setw(12) << left << "Codigo"
         << setw(28) << left << "Nombre"
         << setw(10) << right << "Unidades" << "\n";
    cout << "----------------------------------------\n";

    bool hayDatos = false;
    for (int i = 0; i < TOP_N; i++) {
        if (topIds[i] != 0) {
            Repuesto r;
            if (obtenerRepuestoPorID(rutaRepuestos, topIds[i], r)) {
                cout << setw(5) << left << (i + 1)
                     << setw(12) << left << r.codigo
                     << setw(28) << left << string(r.nombre).substr(0, 27)
                     << setw(10) << right << topUnidades[i] << "\n";
                hayDatos = true;
            }
        }
    }

    if (!hayDatos) {
        cout << "No hay consumo registrado en ordenes cerradas.\n";
    }
    cout << "========================================\n";
}

void reporteFacturacionPorMecanico(const string& rutaMecanicos,
                                   const string& rutaOrdenes) {
    ArchivoHeader headerMec;
    ArchivoHeader headerOrd;
    if (!leerHeader(rutaMecanicos, headerMec) || !leerHeader(rutaOrdenes, headerOrd)) return;

    cout << "\n==================================================================\n";
    cout << "               FACTURACION POR MECANICO\n";
    cout << "==================================================================\n";
    cout << setw(28) << left << "Mecanico"
         << setw(10) << right << "Ordenes"
         << setw(14) << right << "M. obra"
         << setw(14) << right << "Repuestos" << "\n";
    cout << "------------------------------------------------------------------\n";

    ifstream fMec(rutaMecanicos, ios::binary);
    if (!fMec.is_open()) return;

    for (int i = 0; i < headerMec.cantidadRegistros; i++) {
        fMec.seekg(calcularPosicionRegistro(i, sizeof(Mecanico)), ios::beg);
        Mecanico m;
        fMec.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));

        int ordenesCerradas = 0;
        float totalManoObra = 0.0f;
        float totalRepuestos = 0.0f;

        ifstream fOrd(rutaOrdenes, ios::binary);
        if (fOrd.is_open()) {
            fOrd.seekg(sizeof(ArchivoHeader), ios::beg);
            OrdenServicio o;
            for (int j = 0; j < headerOrd.cantidadRegistros; j++) {
                fOrd.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
                if (o.idMecanico == m.id && strcmp(o.estado, "CERRADA") == 0) {
                    ordenesCerradas++;
                    totalManoObra += o.manoObra;
                    for (int k = 0; k < o.numDetalles; k++) {
                        totalRepuestos += o.detalles[k].cantidad * o.detalles[k].precioUnitario;
                    }
                }
            }
            fOrd.close();
        }

        string etiquetaNombre = m.nombre;
        if (m.eliminado) {
            etiquetaNombre += " (BAJA)";
        }

        cout << setw(28) << left << etiquetaNombre.substr(0, 27)
             << setw(10) << right << ordenesCerradas
             << setw(14) << right << fixed << setprecision(2) << totalManoObra
             << setw(14) << right << fixed << setprecision(2) << totalRepuestos << "\n";
    }

    fMec.close();
    cout << "==================================================================\n";
}