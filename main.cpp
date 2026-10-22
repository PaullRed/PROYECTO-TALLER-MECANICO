#include <iostream>
#include <cstring>
#include <iomanip>
#include "Sistema.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include "OrdenServicio.h"

using namespace std;

void mostrarVehiculoConsola(Vehiculo* v) {
    if (v == nullptr) return;
    cout << "ID: " << v->id << "\n";
    cout << "Placa: " << v->placa << "\n";
    cout << "Marca: " << v->marca << "\n";
    cout << "Modelo: " << v->modelo << "\n";
    cout << "Anio: " << v->anio << "\n";
    cout << "Kilometraje: " << v->kilometraje << "\n";
    cout << "Propietario: " << v->nombrePropietario << "\n";
    cout << "Cedula: " << v->cedulaPropietario << "\n";
    cout << "Telefono: " << v->telefonoPropietario << "\n";
    cout << "Fecha Registro: " << v->fechaRegistro << "\n";
}

void mostrarMecanicoConsola(SistemaTaller* s, Mecanico* m) {
    if (m == nullptr) return;
    int abiertas = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idMecanico == m->id && strcmp(s->ordenes[i].estado, "ABIERTA") == 0) {
            abiertas++;
        }
    }
    cout << "ID: " << m->id << " | Nombre: " << m->nombre << " | Cedula: " << m->cedula << " | Especialidad: " << m->especialidad << " | Ordenes Abiertas: " << abiertas << "\n";
}

void mostrarRepuestoConsola(Repuesto* r) {
    if (r == nullptr) return;
    cout << "ID: " << r->id << " | Codigo: " << r->codigo << " | Nombre: " << r->nombre << " | Marca: " << r->marca << " | Precio: $" << fixed << setprecision(2) << r->precio << " | Stock: " << r->stock << " (Minimo: " << r->stockMinimo << ")";
    if (r->stock <= r->stockMinimo) {
        cout << " [BAJO STOCK]";
    }
    cout << "\n";
}

void mostrarDetalleOrdenConsola(SistemaTaller* s, OrdenServicio* o) {
    if (o == nullptr) return;
    Vehiculo* v = buscarVehiculoPorID(s, o->idVehiculo);
    Mecanico* m = buscarMecanicoPorID(s, o->idMecanico);

    cout << "\n========================================\n";
    cout << "          ORDEN DE SERVICIO #" << o->id << "\n";
    cout << "========================================\n";
    cout << "Estado     : " << o->estado << "\n";
    cout << "Apertura   : " << o->fechaApertura << "\n";
    if (strcmp(o->estado, "CERRADA") == 0) {
        cout << "Cierre     : " << o->fechaCierre << "\n";
    }
    if (v != nullptr) {
        cout << "Vehiculo   : " << v->placa << " - " << v->marca << " " << v->modelo << " " << v->anio << " (" << o->kilometrajeIngreso << " km)\n";
        cout << "Dueno      : " << v->nombrePropietario << "\n";
    }
    if (m != nullptr) {
        cout << "Mecanico   : " << m->nombre << " (" << m->especialidad << ")\n";
    }
    cout << "Falla      : " << o->descripcionFalla << "\n";
    cout << "----------------------------------------\n";
    cout << setw(10) << left << "Codigo" << setw(25) << left << "Repuesto" << setw(6) << right << "Cant" << setw(10) << right << "P.Unit" << setw(10) << right << "Total" << "\n";
    cout << "----------------------------------------\n";

    float costoRepuestos = 0.0f;
    for (int i = 0; i < o->numDetalles; i++) {
        Repuesto* r = buscarRepuestoPorID(s, o->detalles[i].idRepuesto);
        char cod[20] = "N/D";
        char nom[100] = "N/D";
        if (r != nullptr) {
            strcpy(cod, r->codigo);
            strcpy(nom, r->nombre);
        }
        float sub = o->detalles[i].cantidad * o->detalles[i].precioUnitario;
        costoRepuestos += sub;
        cout << setw(10) << left << cod 
             << setw(25) << left << nom 
             << setw(6) << right << o->detalles[i].cantidad 
             << setw(10) << right << fixed << setprecision(2) << o->detalles[i].precioUnitario 
             << setw(10) << right << fixed << setprecision(2) << sub << "\n";
    }
    cout << "----------------------------------------\n";
    cout << setw(41) << right << "Repuestos: $" << setw(10) << right << fixed << setprecision(2) << costoRepuestos << "\n";
    cout << setw(41) << right << "Mano de obra: $" << setw(10) << right << fixed << setprecision(2) << o->manoObra << "\n";
    cout << setw(41) << right << "TOTAL: $" << setw(10) << right << fixed << setprecision(2) << calcularTotalOrden(*o) << "\n";
    cout << "========================================\n\n";
}

void menuVehiculos(SistemaTaller* s) {
    int op;
    do {
        cout << "\n--- GESTION DE VEHICULOS ---\n";
        cout << "1. Registrar vehiculo\n";
        cout << "2. Buscar vehiculo por ID o placa\n";
        cout << "3. Buscar vehiculos por propietario\n";
        cout << "4. Actualizar vehiculo\n";
        cout << "5. Eliminar vehiculo\n";
        cout << "6. Listar todos los vehiculos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";
        
        if (!(cin >> op)) {
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
            cout << "Fecha de registro (YYYY-MM-DD): "; cin >> v.fechaRegistro;

            char conf;
            cout << "Desea guardar este registro? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                Vehiculo* res = agregarVehiculo(s, v);
                if (res != nullptr) {
                    cout << "Vehiculo registrado exitosamente con ID " << res->id << ".\n";
                } else {
                    cout << "ERROR: No se pudo registrar el vehiculo (placa duplicada o datos invalidos).\n";
                }
            } else {
                cout << "Operacion cancelada.\n";
            }
        } else if (op == 2) {
            int subop;
            cout << "Buscar por: 1. ID | 2. Placa: "; cin >> subop;
            if (subop == 1) {
                int id;
                cout << "Ingrese ID: "; cin >> id;
                Vehiculo* v = buscarVehiculoPorID(s, id);
                if (v != nullptr) mostrarVehiculoConsola(v);
                else cout << "Vehiculo no encontrado.\n";
            } else {
                char placa[10];
                cout << "Ingrese placa: "; cin >> placa;
                Vehiculo* v = buscarVehiculoPorPlaca(s, placa);
                if (v != nullptr) mostrarVehiculoConsola(v);
                else cout << "Vehiculo no encontrado.\n";
            }
        } else if (op == 3) {
            char sub[100];
            cin.ignore();
            cout << "Ingrese parte del nombre del propietario: "; cin.getline(sub, 100);
            int cant = 0;
            Vehiculo** lista = buscarVehiculosPorPropietario(s, sub, &cant);
            if (lista != nullptr) {
                cout << "\nSe encontraron " << cant << " vehiculos:\n";
                for (int i = 0; i < cant; i++) {
                    cout << "--------------------\n";
                    mostrarVehiculoConsola(lista[i]);
                }
                delete[] lista;
            } else {
                cout << "No se encontraron vehiculos con ese propietario.\n";
            }
        } else if (op == 4) {
            int id;
            cout << "Ingrese ID del vehiculo a actualizar: "; cin >> id;
            Vehiculo* actual = buscarVehiculoPorID(s, id);
            if (actual == nullptr) {
                cout << "Vehiculo no encontrado.\n";
            } else {
                Vehiculo v = *actual;
                cout << "Nueva Marca: "; cin >> v.marca;
                cout << "Nuevo Modelo: "; cin >> v.modelo;
                cout << "Nuevo Anio: "; cin >> v.anio;
                cin.ignore();
                cout << "Nuevo Propietario: "; cin.getline(v.nombrePropietario, 100);
                cout << "Nueva Cedula: "; cin >> v.cedulaPropietario;
                cout << "Nuevo Telefono: "; cin >> v.telefonoPropietario;

                char conf;
                cout << "Confirmar actualizacion? (S/N): "; cin >> conf;
                if (conf == 'S' || conf == 's') {
                    if (actualizarVehiculo(s, id, v)) {
                        cout << "Vehiculo actualizado exitosamente.\n";
                    } else {
                        cout << "ERROR: Datos invalidos.\n";
                    }
                }
            }
        } else if (op == 5) {
            int id;
            cout << "Ingrese ID del vehiculo a eliminar: "; cin >> id;
            char conf;
            cout << "Confirmar eliminacion? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                if (eliminarVehiculo(s, id)) {
                    cout << "Vehiculo eliminado correctamente.\n";
                } else {
                    cout << "ERROR: El vehiculo no existe o tiene ordenes de servicio asociadas.\n";
                }
            }
        } else if (op == 6) {
            int cant = 0;
            Vehiculo** lista = listarVehiculos(s, &cant);
            if (lista != nullptr) {
                cout << "\n--- LISTADO DE VEHICULOS (" << cant << ") ---\n";
                for (int i = 0; i < cant; i++) {
                    cout << "--------------------\n";
                    mostrarVehiculoConsola(lista[i]);
                }
                delete[] lista;
            } else {
                cout << "No hay vehiculos registrados.\n";
            }
        }
    } while (op != 0);
}

void menuMecanicos(SistemaTaller* s) {
    int op;
    do {
        cout << "\n--- GESTION DE MECANICOS ---\n";
        cout << "1. Registrar mecanico\n";
        cout << "2. Buscar mecanico por ID\n";
        cout << "3. Listar todos los mecanicos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";
        
        if (!(cin >> op)) {
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

            char conf;
            cout << "Guardar mecanico? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                Mecanico* res = agregarMecanico(s, m);
                if (res != nullptr) {
                    cout << "Mecanico registrado con ID " << res->id << ".\n";
                } else {
                    cout << "ERROR: Cedula duplicada o especialidad invalida.\n";
                }
            }
        } else if (op == 2) {
            int id;
            cout << "Ingrese ID del mecanico: "; cin >> id;
            Mecanico* m = buscarMecanicoPorID(s, id);
            if (m != nullptr) mostrarMecanicoConsola(s, m);
            else cout << "Mecanico no encontrado.\n";
        } else if (op == 3) {
            int cant = 0;
            Mecanico** lista = listarMecanicos(s, &cant);
            if (lista != nullptr) {
                cout << "\n--- LISTADO DE MECANICOS (" << cant << ") ---\n";
                for (int i = 0; i < cant; i++) {
                    mostrarMecanicoConsola(s, lista[i]);
                }
                delete[] lista;
            } else {
                cout << "No hay mecanicos registrados.\n";
            }
        }
    } while (op != 0);
}

void menuRepuestos(SistemaTaller* s) {
    int op;
    do {
        cout << "\n--- INVENTARIO DE REPUESTOS ---\n";
        cout << "1. Registrar repuesto\n";
        cout << "2. Buscar repuesto\n";
        cout << "3. Registrar entrada de mercancia\n";
        cout << "4. Cambiar precio de repuesto\n";
        cout << "5. Eliminar repuesto\n";
        cout << "6. Listar repuestos\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";
        
        if (!(cin >> op)) {
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
            cout << "Fecha de registro (YYYY-MM-DD): "; cin >> r.fechaRegistro;

            char conf;
            cout << "Guardar repuesto? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                Repuesto* res = agregarRepuesto(s, r);
                if (res != nullptr) {
                    cout << "Repuesto registrado con ID " << res->id << ".\n";
                } else {
                    cout << "ERROR: Codigo duplicado o valores invalidos.\n";
                }
            }
        } else if (op == 2) {
            int subop;
            cout << "Buscar por: 1. ID | 2. Codigo | 3. Nombre parcial: "; cin >> subop;
            if (subop == 1) {
                int id; cout << "Ingrese ID: "; cin >> id;
                Repuesto* r = buscarRepuestoPorID(s, id);
                if (r != nullptr) mostrarRepuestoConsola(r);
                else cout << "No encontrado.\n";
            } else if (subop == 2) {
                char cod[20]; cout << "Ingrese codigo: "; cin >> cod;
                Repuesto* r = buscarRepuestoPorCodigo(s, cod);
                if (r != nullptr) mostrarRepuestoConsola(r);
                else cout << "No encontrado.\n";
            } else {
                char sub[100]; cin.ignore(); cout << "Ingrese nombre parcial: "; cin.getline(sub, 100);
                int cant = 0;
                Repuesto** lista = buscarRepuestosPorNombre(s, sub, &cant);
                if (lista != nullptr) {
                    for (int i = 0; i < cant; i++) mostrarRepuestoConsola(lista[i]);
                    delete[] lista;
                } else cout << "No se encontraron repuestos.\n";
            }
        } else if (op == 3) {
            int id, cant;
            cout << "ID del repuesto: "; cin >> id;
            cout << "Cantidad a ingresar: "; cin >> cant;
            if (registrarEntradaMercancia(s, id, cant)) {
                cout << "Entrada de mercancia registrada con exito.\n";
            } else {
                cout << "ERROR: Repuesto no encontrado o cantidad invalida.\n";
            }
        } else if (op == 4) {
            int id; float nuevoP;
            cout << "ID del repuesto: "; cin >> id;
            cout << "Nuevo precio: "; cin >> nuevoP;
            if (cambiarPrecioRepuesto(s, id, nuevoP)) {
                cout << "Precio actualizado con exito.\n";
            } else {
                cout << "ERROR: Repuesto no encontrado o precio invalido.\n";
            }
        } else if (op == 5) {
            int id;
            cout << "ID del repuesto a eliminar: "; cin >> id;
            char conf; cout << "Confirmar eliminacion? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                if (eliminarRepuesto(s, id)) {
                    cout << "Repuesto eliminado correctamente.\n";
                } else {
                    cout << "ERROR: No existe o esta siendo usado en alguna orden.\n";
                }
            }
        } else if (op == 6) {
            int cant = 0;
            Repuesto** lista = listarRepuestos(s, &cant);
            if (lista != nullptr) {
                cout << "\n--- LISTADO DE REPUESTOS (" << cant << ") ---\n";
                for (int i = 0; i < cant; i++) {
                    mostrarRepuestoConsola(lista[i]);
                }
                delete[] lista;
            } else {
                cout << "No hay repuestos registrados.\n";
            }
        }
    } while (op != 0);
}

void menuOrdenes(SistemaTaller* s) {
    int op;
    do {
        cout << "\n--- ORDENES DE SERVICIO ---\n";
        cout << "1. Abrir orden\n";
        cout << "2. Agregar repuesto a orden\n";
        cout << "3. Quitar repuesto de orden\n";
        cout << "4. Cerrar orden\n";
        cout << "5. Anular orden\n";
        cout << "6. Ver detalle de orden\n";
        cout << "7. Listar ordenes\n";
        cout << "0. Volver\n";
        cout << "Seleccione una opcion: ";
        
        if (!(cin >> op)) {
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

            char conf; cout << "Abrir orden? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                OrdenServicio* o = abrirOrden(s, idVeh, idMec, km, falla, fecha);
                if (o != nullptr) {
                    cout << "Orden abierta exitosamente con ID #" << o->id << ".\n";
                } else {
                    cout << "ERROR: No se pudo abrir la orden (vehiculo o mecanico invalido, vehiculo con orden abierta, mecanico con 3 ordenes abiertas, o kilometraje menor).\n";
                }
            }
        } else if (op == 2) {
            int idOrd, idRep, cant;
            cout << "ID de la orden: "; cin >> idOrd;
            cout << "ID del repuesto: "; cin >> idRep;
            cout << "Cantidad: "; cin >> cant;
            if (agregarRepuestoAOrden(s, idOrd, idRep, cant)) {
                cout << "Repuesto agregado a la orden exitosamente.\n";
            } else {
                cout << "ERROR: Orden no abierta, repuesto no encontrado o stock insuficiente.\n";
            }
        } else if (op == 3) {
            int idOrd, idRep;
            cout << "ID de la orden: "; cin >> idOrd;
            cout << "ID del repuesto a quitar: "; cin >> idRep;
            if (quitarRepuestoDeOrden(s, idOrd, idRep)) {
                cout << "Repuesto removido de la orden y stock devuelto.\n";
            } else {
                cout << "ERROR: No se pudo quitar el repuesto.\n";
            }
        } else if (op == 4) {
            int idOrd; float manoObra; char fechaCierre[11];
            cout << "ID de la orden a cerrar: "; cin >> idOrd;
            cout << "Costo de mano de obra: "; cin >> manoObra;
            cout << "Fecha de cierre (YYYY-MM-DD): "; cin >> fechaCierre;

            char conf; cout << "Confirmar cierre de orden? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                if (cerrarOrden(s, idOrd, manoObra, fechaCierre)) {
                    cout << "Orden cerrada exitosamente.\n";
                } else {
                    cout << "ERROR: Orden no encontrada, no esta abierta, o no tiene detalles ni mano de obra.\n";
                }
            }
        } else if (op == 5) {
            int idOrd;
            cout << "ID de la orden a anular: "; cin >> idOrd;
            char conf; cout << "Confirmar anulacion? (S/N): "; cin >> conf;
            if (conf == 'S' || conf == 's') {
                if (anularOrden(s, idOrd)) {
                    cout << "Orden anulada y stock revertido correctamente.\n";
                } else {
                    cout << "ERROR: Orden no encontrada o ya anulada.\n";
                }
            }
        } else if (op == 6) {
            int idOrd;
            cout << "ID de la orden: "; cin >> idOrd;
            OrdenServicio* o = buscarOrdenPorID(s, idOrd);
            if (o != nullptr) {
                mostrarDetalleOrdenConsola(s, o);
            } else {
                cout << "Orden no encontrada.\n";
            }
        } else if (op == 7) {
            int subop;
            cout << "Listar por: 1. Estado (ABIERTA/CERRADA/ANULADA) | 2. Historial de vehiculo: "; cin >> subop;
            if (subop == 1) {
                char est[15];
                cout << "Ingrese estado: "; cin >> est;
                int cant = 0;
                OrdenServicio** lista = listarOrdenesPorEstado(s, est, &cant);
                if (lista != nullptr) {
                    cout << "\n--- ORDENES CON ESTADO " << est << " (" << cant << ") ---\n";
                    for (int i = 0; i < cant; i++) {
                        mostrarDetalleOrdenConsola(s, lista[i]);
                    }
                    delete[] lista;
                } else {
                    cout << "No hay ordenes con ese estado.\n";
                }
            } else {
                int idVeh;
                cout << "ID del vehiculo: "; cin >> idVeh;
                int cant = 0;
                OrdenServicio** lista = listarHistorialVehiculo(s, idVeh, &cant);
                if (lista != nullptr) {
                    cout << "\n--- HISTORIAL DE ORDENES DEL VEHICULO (" << cant << ") ---\n";
                    for (int i = 0; i < cant; i++) {
                        mostrarDetalleOrdenConsola(s, lista[i]);
                    }
                    delete[] lista;
                } else {
                    cout << "No hay ordenes para este vehiculo.\n";
                }
            }
        }
    } while (op != 0);
}

int main() {
    SistemaTaller sistema;
    Taller t;
    strcpy(t.nombre, "Autoservicio El Lago");
    strcpy(t.rif, "J-12345678-9");
    strcpy(t.direccion, "Av. 5 de Julio, Maracaibo");
    strcpy(t.telefono, "0412-1234567");

    inicializarSistema(&sistema, t);

    int opcion = 0;
    do {
        cout << "\n========================================\n";
        cout << "      SISTEMA DE GESTION DE TALLER\n";
        cout << "      Taller: " << sistema.taller.nombre << "\n";
        cout << "========================================\n";
        cout << "1. Gestion de Vehiculos\n";
        cout << "2. Gestion de Mecanicos\n";
        cout << "3. Inventario de Repuestos\n";
        cout << "4. Ordenes de Servicio\n";
        cout << "0. Salir\n";
        cout << "Seleccione una opcion: ";
        
        if (!(cin >> opcion)) {
            opcion = 0;
            break;
        }

        switch (opcion) {
            case 1:
                menuVehiculos(&sistema);
                break;
            case 2:
                menuMecanicos(&sistema);
                break;
            case 3:
                menuRepuestos(&sistema);
                break;
            case 4:
                menuOrdenes(&sistema);
                break;
            case 0: {
                char conf;
                cout << "Seguro que desea salir? (Se perderan los datos en memoria) (S/N): ";
                cin >> conf;
                if (conf != 'S' && conf != 's') {
                    opcion = -1; 
                } else {
                    cout << "\nSaliendo del sistema...\n";
                }
                break;
            }
            default:
                cout << "\nOpcion no valida. Intente de nuevo.\n";
        }
    } while (opcion != 0);

    liberarSistema(&sistema);
    return 0;
}