#include "Vehiculo.h"
#include <cstring>
#include <cctype>

void aMinuscula(const char* origen, char* destino, int maxLen) {
    int i = 0;
    while (origen[i] != '\0' && i < maxLen - 1) {
        destino[i] = tolower(origen[i]);
        i++;
    }
    destino[i] = '\0';
}

bool contieneSubcadena(const char* texto, const char* subcadena) {
    char t[100];
    char s[100];
    aMinuscula(texto, t, 100);
    aMinuscula(subcadena, s, 100);
    return strstr(t, s) != nullptr;
}

Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos) {
    if (datos.anio < 1950 || datos.anio > 2027 || datos.kilometraje < 0) {
        return nullptr;
    }

    for (int i = 0; i < s->numVehiculos; i++) {
        if (strcmp(s->vehiculos[i].placa, datos.placa) == 0) {
            return nullptr;
        }
    }

    if (s->numVehiculos == s->capacidadVehiculos) {
        int nuevaCapacidad = s->capacidadVehiculos * 2;
        Vehiculo* nuevoArreglo = new Vehiculo[nuevaCapacidad];
        for (int i = 0; i < s->numVehiculos; i++) {
            nuevoArreglo[i] = s->vehiculos[i];
        }
        delete[] s->vehiculos;
        s->vehiculos = nuevoArreglo;
        s->capacidadVehiculos = nuevaCapacidad;
    }

    datos.id = s->siguienteIdVehiculo++;
    s->vehiculos[s->numVehiculos] = datos;
    s->numVehiculos++;

    return &s->vehiculos[s->numVehiculos - 1];
}

Vehiculo* buscarVehiculoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numVehiculos; i++) {
        if (s->vehiculos[i].id == id) {
            return &s->vehiculos[i];
        }
    }
    return nullptr;
}

Vehiculo* buscarVehiculoPorPlaca(SistemaTaller* s, const char* placa) {
    for (int i = 0; i < s->numVehiculos; i++) {
        if (strcmp(s->vehiculos[i].placa, placa) == 0) {
            return &s->vehiculos[i];
        }
    }
    return nullptr;
}

Vehiculo** buscarVehiculosPorPropietario(SistemaTaller* s, const char* subcadena, int* cantidad) {
    int contador = 0;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (contieneSubcadena(s->vehiculos[i].nombrePropietario, subcadena)) {
            contador++;
        }
    }

    *cantidad = contador;
    if (contador == 0) {
        return nullptr;
    }

    Vehiculo** resultado = new Vehiculo*[contador];
    int idx = 0;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (contieneSubcadena(s->vehiculos[i].nombrePropietario, subcadena)) {
            resultado[idx++] = &s->vehiculos[i];
        }
    }

    return resultado;
}

Vehiculo** listarVehiculos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numVehiculos;
    if (s->numVehiculos == 0) {
        return nullptr;
    }

    Vehiculo** resultado = new Vehiculo*[s->numVehiculos];
    for (int i = 0; i < s->numVehiculos; i++) {
        resultado[i] = &s->vehiculos[i];
    }

    return resultado;
}

bool actualizarVehiculo(SistemaTaller* s, int id, Vehiculo datos) {
    Vehiculo* v = buscarVehiculoPorID(s, id);
    if (v == nullptr) {
        return false;
    }

    if (datos.anio < 1950 || datos.anio > 2027) {
        return false;
    }

    strcpy(v->marca, datos.marca);
    strcpy(v->modelo, datos.modelo);
    v->anio = datos.anio;
    strcpy(v->nombrePropietario, datos.nombrePropietario);
    strcpy(v->cedulaPropietario, datos.cedulaPropietario);
    strcpy(v->telefonoPropietario, datos.telefonoPropietario);

    return true;
}

bool eliminarVehiculo(SistemaTaller* s, int id) {
    int index = -1;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (s->vehiculos[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return false;
    }

    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == id) {
            return false;
        }
    }

    for (int i = index; i < s->numVehiculos - 1; i++) {
        s->vehiculos[i] = s->vehiculos[i + 1];
    }

    s->numVehiculos--;
    return true;
}