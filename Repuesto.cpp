#include "Repuesto.h"
#include <cstring>
#include <cctype>

static void aMinusculaR(const char* origen, char* destino, int maxLen) {
    int i = 0;
    while (origen[i] != '\0' && i < maxLen - 1) {
        destino[i] = tolower(origen[i]);
        i++;
    }
    destino[i] = '\0';
}

static bool contieneSubcadenaR(const char* texto, const char* subcadena) {
    char t[100];
    char s[100];
    aMinusculaR(texto, t, 100);
    aMinusculaR(subcadena, s, 100);
    return strstr(t, s) != nullptr;
}

Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos) {
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockMinimo < 0) {
        return nullptr;
    }

    for (int i = 0; i < s->numRepuestos; i++) {
        if (strcmp(s->repuestos[i].codigo, datos.codigo) == 0) {
            return nullptr;
        }
    }

    if (s->numRepuestos == s->capacidadRepuestos) {
        int nuevaCapacidad = s->capacidadRepuestos * 2;
        Repuesto* nuevoArreglo = new Repuesto[nuevaCapacidad];
        for (int i = 0; i < s->numRepuestos; i++) {
            nuevoArreglo[i] = s->repuestos[i];
        }
        delete[] s->repuestos;
        s->repuestos = nuevoArreglo;
        s->capacidadRepuestos = nuevaCapacidad;
    }

    datos.id = s->siguienteIdRepuesto++;
    s->repuestos[s->numRepuestos] = datos;
    s->numRepuestos++;

    return &s->repuestos[s->numRepuestos - 1];
}

Repuesto* buscarRepuestoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numRepuestos; i++) {
        if (s->repuestos[i].id == id) {
            return &s->repuestos[i];
        }
    }
    return nullptr;
}

Repuesto* buscarRepuestoPorCodigo(SistemaTaller* s, const char* codigo) {
    for (int i = 0; i < s->numRepuestos; i++) {
        if (strcmp(s->repuestos[i].codigo, codigo) == 0) {
            return &s->repuestos[i];
        }
    }
    return nullptr;
}

Repuesto** buscarRepuestosPorNombre(SistemaTaller* s, const char* subcadena, int* cantidad) {
    int contador = 0;
    for (int i = 0; i < s->numRepuestos; i++) {
        if (contieneSubcadenaR(s->repuestos[i].nombre, subcadena)) {
            contador++;
        }
    }

    *cantidad = contador;
    if (contador == 0) {
        return nullptr;
    }

    Repuesto** resultado = new Repuesto*[contador];
    int idx = 0;
    for (int i = 0; i < s->numRepuestos; i++) {
        if (contieneSubcadenaR(s->repuestos[i].nombre, subcadena)) {
            resultado[idx++] = &s->repuestos[i];
        }
    }

    return resultado;
}

bool registrarEntradaMercancia(SistemaTaller* s, int idRepuesto, int cantidad) {
    if (cantidad <= 0) {
        return false;
    }
    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr) {
        return false;
    }
    r->stock += cantidad;
    return true;
}

Repuesto** listarRepuestos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numRepuestos;
    if (s->numRepuestos == 0) {
        return nullptr;
    }

    Repuesto** resultado = new Repuesto*[s->numRepuestos];
    for (int i = 0; i < s->numRepuestos; i++) {
        resultado[i] = &s->repuestos[i];
    }

    return resultado;
}

bool cambiarPrecioRepuesto(SistemaTaller* s, int idRepuesto, float nuevoPrecio) {
    if (nuevoPrecio <= 0) {
        return false;
    }
    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr) {
        return false;
    }
    r->precio = nuevoPrecio;
    return true;
}

bool eliminarRepuesto(SistemaTaller* s, int id) {
    int index = -1;
    for (int i = 0; i < s->numRepuestos; i++) {
        if (s->repuestos[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return false;
    }

    for (int i = 0; i < s->numOrdenes; i++) {
        for (int j = 0; j < s->ordenes[i].numDetalles; j++) {
            if (s->ordenes[i].detalles[j].idRepuesto == id) {
                return false;
            }
        }
    }

    for (int i = index; i < s->numRepuestos - 1; i++) {
        s->repuestos[i] = s->repuestos[i + 1];
    }

    s->numRepuestos--;
    return true;
}