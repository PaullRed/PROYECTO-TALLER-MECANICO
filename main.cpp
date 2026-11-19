#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <limits>
#include "Estructura.h"
#include "Persistencia.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include "OrdenServicio.h"
#include "Reportes.h"
#include "Mantenimiento.h"

using namespace std;

const string RUTA_TALLER = "datos/taller.bin";
const string RUTA_VEHICULOS = "datos/vehiculos.bin";
const string RUTA_MECANICOS = "datos/mecanicos.bin";
const string RUTA_REPUESTOS = "datos/repuestos.bin";
const string RUTA_ORDENES = "datos/ordenes.bin";
const int MAX_RESULTADOS_BUSQUEDA = 20;

namespace {
void limpiarEntrada() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int contarOrdenesAbiertasPorMecanico(int idMecanico) {
    ArchivoHeader hOrd;
    if (!leerHeader(RUTA_ORDENES, hOrd)) {
        return 0;
    }

    ifstream archivo(RUTA_ORDENES, ios::binary);
    if (!archivo.is_open()) {
        return 0;
    }

    int abiertas = 0;
    OrdenServicio oTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < hOrd.cantidadRegistros; i++) {
        archivo.seekg(pos, ios::beg);
        archivo.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
        if (oTemp.idMecanico == idMecanico && strcmp(oTemp.estado, "ABIERTA") == 0) {
            abiertas++;
        }
        pos += sizeof(OrdenServicio);
    }
    archivo.close();
    return abiertas;
}
} // namespace

void inicializarSistemaArchivos() {
    ifstream fTaller(RUTA_TALLER, ios::binary);
    if (!fTaller.is_open()) {
        ofstream fNuevo(RUTA_TALLER, ios::binary);
        if (fNuevo.is_open()) {
            Taller t;
            strcpy(t.nombre, "Autoservicio El Lago");
            strcpy(t.rif, "J-12345678-9");
            strcpy(t.direccion, "Av. 5 de Julio, Maracaibo");
            strcpy(t.telefono, "0412-1234567");
            t.fechaCreacion = time(nullptr);
            t.fechaUltimaModificacion = t.fechaCreacion;
            fNuevo.write(reinterpret_cast<const char*>(&t), sizeof(Taller));
            fNuevo.close();
        }
    } else {
        fTaller.close();
    }

    inicializarArchivoEntidad(RUTA_VEHICULOS);
    inicializarArchivoEntidad(RUTA_MECANICOS);
    inicializarArchivoEntidad(RUTA_REPUESTOS);
    inicializarArchivoEntidad(RUTA_ORDENES);
}

void mostrarVehiculoConsola(const Vehiculo& v) {
    cout << "ID: " << v.id << (v.eliminado ? " [ELIMINADO]" : "") << "\n";
    cout << "Placa: " << v.placa << "\n";
    cout << "Marca: " << v.marca << "\n";
    cout << "Modelo: " << v.modelo << "\n";
    cout << "Anio: " << v.anio << "\n";
    cout << "Kilometraje: " << v.kilometraje << "\n";
    cout << "Propietario: " << v.nombrePropietario << "\n";
    cout << "Cedula: " << v.cedulaPropietario << "\n";
    cout << "Telefono: " << v.telefonoPropietario << "\n";
}

void mostrarMecanicoConsola(const Mecanico& m) {
    int abiertas = contarOrdenesAbiertasPorMecanico(m.id);
    cout << "ID: " << m.id
         << " | Nombre: " << m.nombre
         << " | Cedula: " << m.cedula
         << " | Especialidad: " << m.especialidad
         << " | Ordenes ABIERTAS: " << abiertas
         << (m.eliminado ? " [BAJA]" : " [ACTIVO]") << "\n";
}

void mostrarRepuestoConsola(const Repuesto& r) {
    cout << "ID: " << r.id
         << " | Codigo: " << r.codigo
         << " | Nombre: " << r.nombre
         << " | Marca: " << r.marca
         << " | Precio: $" << fixed << setprecision(2) << r.precio
         << " | Stock: " << r.stock
         << " (Minimo: " << r.stockMinimo << ")";
    if (r.stock <= r.stockMinimo) {
        cout << " [BAJO STOCK]";
    }
    cout << "\n";
}

void mostrarDetalleOrdenConsola(const OrdenServicio& o) {
    Vehiculo v;
    Mecanico m;
    obtenerVehiculoPorID(RUTA_VEHICULOS, o.idVehiculo, v);
    obtenerMecanicoPorID(RUTA_MECANICOS, o.idMecanico, m);

    cout << "\n========================================\n";
    cout << "          ORDEN DE SERVICIO #" << o.id << "\n";
    cout << "========================================\n";
    cout << "Estado     : " << o.estado << "\n";
    cout << "Apertura   : " << o.fechaApertura << "\n";
    if (strcmp(o.estado, "CERRADA") == 0) {
        cout << "Cierre     : " << o.fechaCierre << "\n";
    }
    cout << "Vehiculo   : " << v.placa << " - " << v.marca << " " << v.modelo << " " << v.anio << " (" << o.kilometrajeIngreso << " km)\n";
    cout << "Dueno      : " << v.nombrePropietario << "\n";
    cout << "Mecanico   : " << m.nombre << " (" << m.especialidad << (m.eliminado ? ", BAJA" : "") << ")\n";
    cout << "Falla      : " << o.descripcionFalla << "\n";
    cout << "----------------------------------------\n";
    cout << setw(10) << left << "Codigo" << setw(25) << left << "Repuesto" << setw(6) << right << "Cant" << setw(10) << right << "P.Unit" << setw(10) << right << "Total" << "\n";
    cout << "----------------------------------------\n";

    float costoRepuestos = 0.0f;
    for (int i = 0; i < o.numDetalles; i++) {
        Repuesto r;
        obtenerRepuestoPorID(RUTA_REPUESTOS, o.detalles[i].idRepuesto, r);
        float sub = o.detalles[i].cantidad * o.detalles[i].precioUnitario;
        costoRepuestos += sub;
        cout << setw(10) << left << r.codigo
             << setw(25) << left << r.nombre
             << setw(6) << right << o.detalles[i].cantidad
             << setw(10) << right << fixed << setprecision(2) << o.detalles[i].precioUnitario
             << setw(10) << right << fixed << setprecision(2) << sub << "\n";
    }
    cout << "----------------------------------------\n";
    cout << setw(41) << right << "Repuestos: $" << setw(10) << right << fixed << setprecision(2) << costoRepuestos << "\n";
    cout << setw(41) << right << "Mano de obra: $" << setw(10) << right << fixed << setprecision(2) << o.manoObra << "\n";
    cout << setw(41) << right << "TOTAL: $" << setw(10) << right << fixed << setprecision(2) << calcularTotalOrdenServicio(o) << "\n";
    cout << "========================================\n\n";
}

void menuVehiculos() {
    int op;
    do {
        cout << "\n--- GESTION DE VEHICULOS ---\n";
        cout << "1. Registrar vehiculo\n";
        cout << "2. Buscar vehiculo por ID, placa o propietario (parcial)\n";
        cout << "3. Actualizar vehiculo\n";
        cout << "4. Eliminar vehiculo\n";
        cout << "5. Listar vehiculos activos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            Vehiculo v;
            cout << "Placa: "; cin >> v.placa;
            cout << "Marca: "; cin >> v.marca;
            cout << "Modelo: "; cin >> v.modelo;
            cout << "Anio: "; cin >> v.anio;
            cout << "Kilometraje: "; cin >> v.kilometraje;
            cin.ignore();
            cout << "Nombre del propietario: "; cin.getline(v.nombrePropietario, 100);
            cout << "Cedula del propietario: "; cin >> v.cedulaPropietario;
            cout << "Telefono del propietario: "; cin >> v.telefonoPropietario;

            char conf;
            cout << "Desea guardar este registro? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                int idAsignado = 0;
                if (registrarVehiculo(RUTA_VEHICULOS, v, idAsignado)) {
                    cout << "Vehiculo registrado exitosamente con ID " << idAsignado << ".\n";
                } else {
                    cout << "ERROR: No se pudo registrar el vehiculo (placa duplicada o datos invalidos).\n";
                }
            }
        } else if (op == 2) {
            int subop;
            cout << "Buscar por: 1. ID | 2. Placa | 3. Propietario (parcial): "; cin >> subop;
            if (subop == 1) {
                int id;
                cout << "Ingrese ID: "; cin >> id;
                Vehiculo v;
                if (!obtenerVehiculoPorID(RUTA_VEHICULOS, id, v)) {
                    cout << "Vehiculo no encontrado (ID inexistente).\n";
                } else if (v.eliminado) {
                    cout << "El vehiculo con ID " << id << " fue eliminado logicamente.\n";
                } else {
                    cout << "[Acceso directo en offset: " << calcularPosicionRegistro(id - 1, sizeof(Vehiculo)) << " bytes]\n";
                    mostrarVehiculoConsola(v);
                }
            } else if (subop == 2) {
                string placa;
                cout << "Ingrese placa: "; cin >> placa;
                Vehiculo v;
                if (obtenerVehiculoPorPlaca(RUTA_VEHICULOS, placa, v)) {
                    mostrarVehiculoConsola(v);
                } else {
                    cout << "Vehiculo no encontrado o eliminado.\n";
                }
            } else if (subop == 3) {
                string fragmento;
                cin.ignore();
                cout << "Ingrese parte del nombre del propietario: ";
                getline(cin, fragmento);
                Vehiculo resultados[MAX_RESULTADOS_BUSQUEDA];
                int encontrados = 0;
                bool hayMas = false;
                if (buscarVehiculosPorPropietarioParcial(RUTA_VEHICULOS, fragmento, resultados,
                                                         MAX_RESULTADOS_BUSQUEDA, encontrados, hayMas)) {
                    if (encontrados == 0) {
                        cout << "No se encontraron vehiculos activos con ese propietario.\n";
                    } else {
                        for (int i = 0; i < encontrados; i++) {
                            cout << "--------------------\n";
                            mostrarVehiculoConsola(resultados[i]);
                        }
                        if (hayMas) {
                            cout << "Aviso: existen mas resultados mas alla del limite (" << MAX_RESULTADOS_BUSQUEDA << ").\n";
                        }
                    }
                } else {
                    cout << "ERROR: Entrada invalida para la busqueda.\n";
                }
            }
        } else if (op == 3) {
            int id;
            cout << "Ingrese ID del vehiculo a actualizar: "; cin >> id;
            Vehiculo v;
            if (!obtenerVehiculoPorID(RUTA_VEHICULOS, id, v) || v.eliminado) {
                cout << "Vehiculo no encontrado o eliminado.\n";
            } else {
                cout << "Nueva Marca: "; cin >> v.marca;
                cout << "Nuevo Modelo: "; cin >> v.modelo;
                cout << "Nuevo Anio: "; cin >> v.anio;
                cin.ignore();
                cout << "Nuevo Propietario: "; cin.getline(v.nombrePropietario, 100);
                cout << "Nueva Cedula: "; cin >> v.cedulaPropietario;
                cout << "Nuevo Telefono: "; cin >> v.telefonoPropietario;

                if (actualizarVehiculo(RUTA_VEHICULOS, id, v)) {
                    cout << "Vehiculo actualizado exitosamente.\n";
                } else {
                    cout << "ERROR: Datos invalidos.\n";
                }
            }
        } else if (op == 4) {
            int id;
            cout << "Ingrese ID del vehiculo a eliminar: "; cin >> id;
            if (eliminarVehiculoLogico(RUTA_VEHICULOS, id, RUTA_ORDENES)) {
                cout << "Vehiculo eliminado correctamente.\n";
            } else {
                cout << "ERROR: El vehiculo no existe, ya esta eliminado o tiene ordenes registradas.\n";
            }
        } else if (op == 5) {
            ArchivoHeader h;
            leerHeader(RUTA_VEHICULOS, h);
            ifstream archivo(RUTA_VEHICULOS, ios::binary);
            if (archivo.is_open()) {
                cout << "\n--- LISTADO DE VEHICULOS ACTIVOS ---\n";
                Vehiculo v;
                long pos = sizeof(ArchivoHeader);
                for (int i = 0; i < h.cantidadRegistros; i++) {
                    archivo.seekg(pos, ios::beg);
                    archivo.read(reinterpret_cast<char*>(&v), sizeof(Vehiculo));
                    if (!v.eliminado) {
                        cout << "--------------------\n";
                        mostrarVehiculoConsola(v);
                    }
                    pos += sizeof(Vehiculo);
                }
                archivo.close();
            }
        }
    } while (op != 0);
}

void menuMecanicos() {
    int op;
    do {
        cout << "\n--- GESTION DE MECANICOS ---\n";
        cout << "1. Registrar mecanico\n";
        cout << "2. Buscar mecanico por ID\n";
        cout << "3. Dar de baja mecanico\n";
        cout << "4. Listar todos los mecanicos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            Mecanico m;
            cin.ignore();
            cout << "Nombre completo: "; cin.getline(m.nombre, 100);
            cout << "Cedula: "; cin >> m.cedula;
            cout << "Especialidad (MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL): "; cin >> m.especialidad;
            cout << "Fecha de ingreso (YYYY-MM-DD): "; cin >> m.fechaIngreso;

            int idAsignado = 0;
            if (registrarMecanico(RUTA_MECANICOS, m, idAsignado)) {
                cout << "Mecanico registrado con ID " << idAsignado << ".\n";
            } else {
                cout << "ERROR: Cedula duplicada o especialidad invalida.\n";
            }
        } else if (op == 2) {
            int id;
            cout << "Ingrese ID del mecanico: "; cin >> id;
            Mecanico m;
            if (obtenerMecanicoPorID(RUTA_MECANICOS, id, m)) {
                cout << "[Acceso directo en offset: " << calcularPosicionRegistro(id - 1, sizeof(Mecanico)) << " bytes]\n";
                mostrarMecanicoConsola(m);
            } else {
                cout << "Mecanico no encontrado.\n";
            }
        } else if (op == 3) {
            int id;
            cout << "Ingrese ID del mecanico a dar de baja: "; cin >> id;
            if (darDeBajaMecanicoLogico(RUTA_MECANICOS, id, RUTA_ORDENES)) {
                cout << "Mecanico dado de baja exitosamente.\n";
            } else {
                cout << "ERROR: Mecanico no encontrado, ya dado de baja o tiene ordenes ABIERTAS.\n";
            }
        } else if (op == 4) {
            ArchivoHeader h;
            leerHeader(RUTA_MECANICOS, h);
            ifstream archivo(RUTA_MECANICOS, ios::binary);
            if (archivo.is_open()) {
                cout << "\n--- LISTADO DE MECANICOS ---\n";
                Mecanico m;
                long pos = sizeof(ArchivoHeader);
                for (int i = 0; i < h.cantidadRegistros; i++) {
                    archivo.seekg(pos, ios::beg);
                    archivo.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));
                    mostrarMecanicoConsola(m);
                    pos += sizeof(Mecanico);
                }
                archivo.close();
            }
        }
    } while (op != 0);
}

void menuRepuestos() {
    int op;
    do {
        cout << "\n--- INVENTARIO DE REPUESTOS ---\n";
        cout << "1. Registrar repuesto\n";
        cout << "2. Buscar repuesto por ID, codigo o nombre (parcial)\n";
        cout << "3. Registrar entrada de mercancia\n";
        cout << "4. Cambiar precio de repuesto\n";
        cout << "5. Eliminar repuesto\n";
        cout << "6. Listar repuestos activos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            Repuesto r;
            cout << "Codigo del repuesto: "; cin >> r.codigo;
            cin.ignore();
            cout << "Nombre del repuesto: "; cin.getline(r.nombre, 100);
            cout << "Marca: "; cin >> r.marca;
            cout << "Precio unitario: "; cin >> r.precio;
            cout << "Stock inicial: "; cin >> r.stock;
            cout << "Stock minimo: "; cin >> r.stockMinimo;

            int idAsignado = 0;
            if (registrarRepuesto(RUTA_REPUESTOS, r, idAsignado)) {
                cout << "Repuesto registrado con ID " << idAsignado << ".\n";
            } else {
                cout << "ERROR: Codigo duplicado o valores invalidos.\n";
            }
        } else if (op == 2) {
            int subop;
            cout << "Buscar por: 1. ID | 2. Codigo | 3. Nombre (parcial): "; cin >> subop;
            if (subop == 1) {
                int id; cout << "Ingrese ID: "; cin >> id;
                Repuesto r;
                if (obtenerRepuestoPorID(RUTA_REPUESTOS, id, r) && !r.eliminado) {
                    cout << "[Acceso directo en offset: " << calcularPosicionRegistro(id - 1, sizeof(Repuesto)) << " bytes]\n";
                    mostrarRepuestoConsola(r);
                } else {
                    cout << "No encontrado o eliminado.\n";
                }
            } else if (subop == 2) {
                string codigo;
                cout << "Ingrese codigo: "; cin >> codigo;
                Repuesto r;
                if (obtenerRepuestoPorCodigo(RUTA_REPUESTOS, codigo, r)) {
                    mostrarRepuestoConsola(r);
                } else {
                    cout << "Repuesto activo no encontrado con ese codigo.\n";
                }
            } else if (subop == 3) {
                string fragmento;
                cin.ignore();
                cout << "Ingrese parte del nombre del repuesto: ";
                getline(cin, fragmento);
                Repuesto resultados[MAX_RESULTADOS_BUSQUEDA];
                int encontrados = 0;
                bool hayMas = false;
                if (buscarRepuestosPorNombreParcial(RUTA_REPUESTOS, fragmento, resultados,
                                                     MAX_RESULTADOS_BUSQUEDA, encontrados, hayMas)) {
                    if (encontrados == 0) {
                        cout << "No se encontraron repuestos activos que coincidan.\n";
                    } else {
                        for (int i = 0; i < encontrados; i++) {
                            mostrarRepuestoConsola(resultados[i]);
                        }
                        if (hayMas) {
                            cout << "Aviso: existen mas resultados mas alla del limite (" << MAX_RESULTADOS_BUSQUEDA << ").\n";
                        }
                    }
                } else {
                    cout << "ERROR: Entrada invalida para la busqueda.\n";
                }
            }
        } else if (op == 3) {
            int id, cant;
            cout << "ID del repuesto: "; cin >> id;
            cout << "Cantidad a ingresar: "; cin >> cant;
            if (registrarEntradaMercancia(RUTA_REPUESTOS, id, cant)) {
                cout << "Entrada de mercancia registrada con exito.\n";
            } else {
                cout << "ERROR: Repuesto no encontrado o cantidad invalida.\n";
            }
        } else if (op == 4) {
            int id; float nuevoP;
            cout << "ID del repuesto: "; cin >> id;
            cout << "Nuevo precio: "; cin >> nuevoP;
            if (cambiarPrecioRepuesto(RUTA_REPUESTOS, id, nuevoP)) {
                cout << "Precio actualizado con exito.\n";
            } else {
                cout << "ERROR: Repuesto no encontrado o precio invalido.\n";
            }
        } else if (op == 5) {
            int id;
            cout << "ID del repuesto a eliminar: "; cin >> id;
            if (eliminarRepuestoLogico(RUTA_REPUESTOS, id, RUTA_ORDENES)) {
                cout << "Repuesto eliminado correctamente.\n";
            } else {
                cout << "ERROR: No existe, ya fue eliminado o esta asociado a una orden.\n";
            }
        } else if (op == 6) {
            ArchivoHeader h;
            leerHeader(RUTA_REPUESTOS, h);
            ifstream archivo(RUTA_REPUESTOS, ios::binary);
            if (archivo.is_open()) {
                cout << "\n--- LISTADO DE REPUESTOS ACTIVOS ---\n";
                Repuesto r;
                long pos = sizeof(ArchivoHeader);
                for (int i = 0; i < h.cantidadRegistros; i++) {
                    archivo.seekg(pos, ios::beg);
                    archivo.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));
                    if (!r.eliminado) {
                        mostrarRepuestoConsola(r);
                    }
                    pos += sizeof(Repuesto);
                }
                archivo.close();
            }
        }
    } while (op != 0);
}

void menuOrdenes() {
    int op;
    do {
        cout << "\n--- ORDENES DE SERVICIO ---\n";
        cout << "1. Abrir orden\n";
        cout << "2. Agregar repuesto a orden\n";
        cout << "3. Quitar repuesto de orden\n";
        cout << "4. Cerrar orden\n";
        cout << "5. Anular orden\n";
        cout << "6. Ver detalle de orden\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            int idVeh, idMec, km;
            char falla[200];
            char fecha[11];
            cout << "ID del vehiculo: "; cin >> idVeh;
            cout << "ID del mecanico: "; cin >> idMec;
            cout << "Kilometraje de ingreso: "; cin >> km;
            cin.ignore();
            cout << "Descripcion de la falla: "; cin.getline(falla, 200);
            cout << "Fecha actual (YYYY-MM-DD): "; cin >> fecha;

            int idAsignado = 0;
            if (abrirOrdenServicio(RUTA_ORDENES, RUTA_VEHICULOS, RUTA_MECANICOS, idVeh, idMec, km, falla, fecha, idAsignado)) {
                cout << "Orden abierta exitosamente con ID #" << idAsignado << ".\n";
            } else {
                cout << "ERROR: No se pudo abrir la orden.\n";
            }
        } else if (op == 2) {
            int idOrd, idRep, cant;
            cout << "ID de la orden: "; cin >> idOrd;
            cout << "ID del repuesto: "; cin >> idRep;
            cout << "Cantidad: "; cin >> cant;
            if (agregarRepuestoAOrden(RUTA_ORDENES, RUTA_REPUESTOS, idOrd, idRep, cant)) {
                cout << "Repuesto agregado a la orden exitosamente.\n";
            } else {
                cout << "ERROR: Orden no abierta, repuesto no encontrado, limite de 15 lineas alcanzado o stock insuficiente.\n";
            }
        } else if (op == 3) {
            int idOrd, idRep;
            cout << "ID de la orden: "; cin >> idOrd;
            cout << "ID del repuesto a quitar: "; cin >> idRep;
            if (quitarRepuestoDeOrden(RUTA_ORDENES, RUTA_REPUESTOS, idOrd, idRep)) {
                cout << "Repuesto removido de la orden y stock devuelto.\n";
            } else {
                cout << "ERROR: No se pudo quitar el repuesto.\n";
            }
        } else if (op == 4) {
            int idOrd; float manoObra; char fechaCierre[11];
            cout << "ID de la orden a cerrar: "; cin >> idOrd;
            cout << "Costo de mano de obra: "; cin >> manoObra;
            cout << "Fecha de cierre (YYYY-MM-DD): "; cin >> fechaCierre;

            if (cerrarOrdenServicio(RUTA_ORDENES, idOrd, manoObra, fechaCierre)) {
                cout << "Orden cerrada exitosamente.\n";
            } else {
                cout << "ERROR: Orden no encontrada, no esta abierta, o datos invalidos.\n";
            }
        } else if (op == 5) {
            int idOrd;
            cout << "ID de la orden a anular: "; cin >> idOrd;
            if (anularOrdenServicio(RUTA_ORDENES, RUTA_REPUESTOS, idOrd)) {
                cout << "Orden anulada y stock revertido correctamente.\n";
            } else {
                cout << "ERROR: Orden no encontrada o ya anulada.\n";
            }
        } else if (op == 6) {
            int idOrd;
            cout << "ID de la orden: "; cin >> idOrd;
            OrdenServicio o;
            if (obtenerOrdenPorID(RUTA_ORDENES, idOrd, o)) {
                mostrarDetalleOrdenConsola(o);
            } else {
                cout << "Orden no encontrada.\n";
            }
        }
    } while (op != 0);
}

void menuReportes() {
    int op;
    do {
        cout << "\n--- REPORTES DEL SISTEMA ---\n";
        cout << "1. Historial de vehiculo por placa\n";
        cout << "2. Top 10 repuestos mas utilizados\n";
        cout << "3. Facturacion por mecanico\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            string placa;
            cout << "Ingrese placa del vehiculo: ";
            cin >> placa;
            reporteHistorialVehiculo(placa, RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_ORDENES);
        } else if (op == 2) {
            reporteRepuestosMasUtilizados(RUTA_REPUESTOS, RUTA_ORDENES);
        } else if (op == 3) {
            reporteFacturacionPorMecanico(RUTA_MECANICOS, RUTA_ORDENES);
        }
    } while (op != 0);
}

void menuMantenimiento() {
    int op;
    do {
        cout << "\n--- MANTENIMIENTO DEL SISTEMA ---\n";
        cout << "1. Verificar integridad de archivos\n";
        cout << "2. Crear respaldo (Backup)\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> op)) {
            limpiarEntrada();
            op = 0;
            break;
        }

        if (op == 1) {
            verificarIntegridad(RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES);
        } else if (op == 2) {
            string carpetaBackup;
            if (crearBackup(RUTA_TALLER, RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES, carpetaBackup)) {
                cout << "Respaldo creado exitosamente en la carpeta: " << carpetaBackup << "/\n";
            } else {
                cout << "ERROR: No se pudo completar el respaldo de todos los archivos.\n";
            }
        }
    } while (op != 0);
}

int main() {
    inicializarSistemaArchivos();

    Taller t;
    ifstream archivoTaller(RUTA_TALLER, ios::binary);
    if (archivoTaller.is_open()) {
        archivoTaller.read(reinterpret_cast<char*>(&t), sizeof(Taller));
        archivoTaller.close();
    }

    int opcion = 0;
    do {
        cout << "\n========================================\n";
        cout << "      SISTEMA DE GESTION DE TALLER\n";
        cout << "      Taller: " << t.nombre << "\n";
        cout << "========================================\n";
        cout << "1. Gestion de Vehiculos\n";
        cout << "2. Gestion de Mecanicos\n";
        cout << "3. Inventario de Repuestos\n";
        cout << "4. Ordenes de Servicio\n";
        cout << "5. Reportes\n";
        cout << "6. Mantenimiento\n";
        cout << "0. Salir\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            limpiarEntrada();
            opcion = 0;
            break;
        }

        switch (opcion) {
            case 1:
                menuVehiculos();
                break;
            case 2:
                menuMecanicos();
                break;
            case 3:
                menuRepuestos();
                break;
            case 4:
                menuOrdenes();
                break;
            case 5:
                menuReportes();
                break;
            case 6:
                menuMantenimiento();
                break;
            case 0:
                cout << "\nSaliendo del sistema...\n";
                break;
            default:
                cout << "\nOpcion no valida. Intente de nuevo.\n";
        }
    } while (opcion != 0);

    return 0;
}