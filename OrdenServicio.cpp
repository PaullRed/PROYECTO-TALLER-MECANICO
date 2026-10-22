#include "OrdenServicio.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include <cstring>

OrdenServicio* abrirOrden(SistemaTaller* s, int idVehiculo, int idMecanico, int kilometrajeIngreso, const char* descripcionFalla, const char* fechaActual) {
    Vehiculo* v = buscarVehiculoPorID(s, idVehiculo);
    if (v == nullptr) {
        return nullptr;
    }

    Mecanico* m = buscarMecanicoPorID(s, idMecanico);
    if (m == nullptr) {
        return nullptr;
    }

    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == idVehiculo && strcmp(s->ordenes[i].estado, "ABIERTA") == 0) {
            return nullptr;
        }
    }

    int ordenesAbiertasMecanico = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idMecanico == idMecanico && strcmp(s->ordenes[i].estado, "ABIERTA") == 0) {
            ordenesAbiertasMecanico++;
        }
    }
    if (ordenesAbiertasMecanico >= 3) {
        return nullptr;
    }

    if (kilometrajeIngreso < v->kilometraje) {
        return nullptr;
    }

    v->kilometraje = kilometrajeIngreso;

    if (s->numOrdenes == s->capacidadOrdenes) {
        int nuevaCapacidad = s->capacidadOrdenes * 2;
        OrdenServicio* nuevoArreglo = new OrdenServicio[nuevaCapacidad];
        for (int i = 0; i < s->numOrdenes; i++) {
            nuevoArreglo[i] = s->ordenes[i];
        }
        delete[] s->ordenes;
        s->ordenes = nuevoArreglo;
        s->capacidadOrdenes = nuevaCapacidad;
    }

    OrdenServicio& nueva = s->ordenes[s->numOrdenes];
    nueva.id = s->siguienteIdOrden++;
    nueva.idVehiculo = idVehiculo;
    nueva.idMecanico = idMecanico;
    strcpy(nueva.fechaApertura, fechaActual);
    strcpy(nueva.fechaCierre, "");
    strcpy(nueva.estado, "ABIERTA");
    strcpy(nueva.descripcionFalla, descripcionFalla);
    nueva.kilometrajeIngreso = kilometrajeIngreso;
    nueva.manoObra = 0.0f;
    nueva.capacidadDetalles = 2;
    nueva.numDetalles = 0;
    nueva.detalles = new DetalleOrden[nueva.capacidadDetalles];

    s->numOrdenes++;
    return &nueva;
}

bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad) {
    if (cantidad <= 0) {
        return false;
    }

    OrdenServicio* orden = buscarOrdenPorID(s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ABIERTA") != 0) {
        return false;
    }

    Repuesto* rep = buscarRepuestoPorID(s, idRepuesto);
    if (rep == nullptr || rep->stock < cantidad) {
        return false;
    }

    int indiceExistente = -1;
    for (int i = 0; i < orden->numDetalles; i++) {
        if (orden->detalles[i].idRepuesto == idRepuesto) {
            indiceExistente = i;
            break;
        }
    }

    if (indiceExistente != -1) {
        orden->detalles[indiceExistente].cantidad += cantidad;
    } else {
        if (orden->numDetalles == orden->capacidadDetalles) {
            int nuevaCap = orden->capacidadDetalles * 2;
            DetalleOrden* nuevoDet = new DetalleOrden[nuevaCap];
            for (int i = 0; i < orden->numDetalles; i++) {
                nuevoDet[i] = orden->detalles[i];
            }
            delete[] orden->detalles;
            orden->detalles = nuevoDet;
            orden->capacidadDetalles = nuevaCap;
        }

        orden->detalles[orden->numDetalles].idRepuesto = idRepuesto;
        orden->detalles[orden->numDetalles].cantidad = cantidad;
        orden->detalles[orden->numDetalles].precioUnitario = rep->precio;
        orden->numDetalles++;
    }

    rep->stock -= cantidad;
    return true;
}

bool quitarRepuestoDeOrden(SistemaTaller* s, int idOrden, int idRepuesto) {
    OrdenServicio* orden = buscarOrdenPorID(s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ABIERTA") != 0) {
        return false;
    }

    int indice = -1;
    for (int i = 0; i < orden->numDetalles; i++) {
        if (orden->detalles[i].idRepuesto == idRepuesto) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        return false;
    }

    Repuesto* rep = buscarRepuestoPorID(s, idRepuesto);
    if (rep != nullptr) {
        rep->stock += orden->detalles[indice].cantidad;
    }

    for (int i = indice; i < orden->numDetalles - 1; i++) {
        orden->detalles[i] = orden->detalles[i + 1];
    }
    orden->numDetalles--;

    return true;
}

bool cerrarOrden(SistemaTaller* s, int idOrden, float manoObra, const char* fechaCierre) {
    if (manoObra < 0) {
        return false;
    }

    OrdenServicio* orden = buscarOrdenPorID(s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ABIERTA") != 0) {
        return false;
    }

    if (orden->numDetalles == 0 && manoObra <= 0) {
        return false;
    }

    orden->manoObra = manoObra;
    strcpy(orden->estado, "CERRADA");
    strcpy(orden->fechaCierre, fechaCierre);

    return true;
}

bool anularOrden(SistemaTaller* s, int idOrden) {
    OrdenServicio* orden = buscarOrdenPorID(s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ANULADA") == 0) {
        return false;
    }

    if (strcmp(orden->estado, "ABIERTA") == 0 || strcmp(orden->estado, "CERRADA") == 0) {
        for (int i = 0; i < orden->numDetalles; i++) {
            Repuesto* rep = buscarRepuestoPorID(s, orden->detalles[i].idRepuesto);
            if (rep != nullptr) {
                rep->stock += orden->detalles[i].cantidad;
            }
        }
    }

    strcpy(orden->estado, "ANULADA");
    return true;
}

float calcularTotalOrden(OrdenServicio orden) {
    float total = orden.manoObra;
    for (int i = 0; i < orden.numDetalles; i++) {
        total += orden.detalles[i].cantidad * orden.detalles[i].precioUnitario;
    }
    return total;
}

OrdenServicio* buscarOrdenPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].id == id) {
            return &s->ordenes[i];
        }
    }
    return nullptr;
}

OrdenServicio** listarOrdenesPorEstado(SistemaTaller* s, const char* estado, int* cantidad) {
    int contador = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (strcmp(s->ordenes[i].estado, estado) == 0) {
            contador++;
        }
    }

    *cantidad = contador;
    if (contador == 0) {
        return nullptr;
    }

    OrdenServicio** resultado = new OrdenServicio*[contador];
    int idx = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (strcmp(s->ordenes[i].estado, estado) == 0) {
            resultado[idx++] = &s->ordenes[i];
        }
    }

    return resultado;
}

OrdenServicio** listarHistorialVehiculo(SistemaTaller* s, int idVehiculo, int* cantidad) {
    int contador = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == idVehiculo) {
            contador++;
        }
    }

    *cantidad = contador;
    if (contador == 0) {
        return nullptr;
    }

    OrdenServicio** resultado = new OrdenServicio*[contador];
    int idx = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == idVehiculo) {
            resultado[idx++] = &s->ordenes[i];
        }
    }

    return resultado;
}