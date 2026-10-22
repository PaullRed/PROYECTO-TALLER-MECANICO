#include "Mecanico.h"
#include <cstring>

bool esEspecialidadValida(const char* esp) {
    return strcmp(esp, "MOTOR") == 0 ||
           strcmp(esp, "FRENOS") == 0 ||
           strcmp(esp, "ELECTRICIDAD") == 0 ||
           strcmp(esp, "SUSPENSION") == 0 ||
           strcmp(esp, "GENERAL") == 0;
}

Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos) {
    if (!esEspecialidadValida(datos.especialidad)) {
        return nullptr;
    }

    for (int i = 0; i < s->numMecanicos; i++) {
        if (strcmp(s->mecanicos[i].cedula, datos.cedula) == 0) {
            return nullptr;
        }
    }

    if (s->numMecanicos == s->capacidadMecanicos) {
        int nuevaCapacidad = s->capacidadMecanicos * 2;
        Mecanico* nuevoArreglo = new Mecanico[nuevaCapacidad];
        for (int i = 0; i < s->numMecanicos; i++) {
            nuevoArreglo[i] = s->mecanicos[i];
        }
        delete[] s->mecanicos;
        s->mecanicos = nuevoArreglo;
        s->capacidadMecanicos = nuevaCapacidad;
    }

    datos.id = s->siguienteIdMecanico++;
    s->mecanicos[s->numMecanicos] = datos;
    s->numMecanicos++;

    return &s->mecanicos[s->numMecanicos - 1];
}

Mecanico* buscarMecanicoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numMecanicos; i++) {
        if (s->mecanicos[i].id == id) {
            return &s->mecanicos[i];
        }
    }
    return nullptr;
}

Mecanico* buscarMecanicoPorCedula(SistemaTaller* s, const char* cedula) {
    for (int i = 0; i < s->numMecanicos; i++) {
        if (strcmp(s->mecanicos[i].cedula, cedula) == 0) {
            return &s->mecanicos[i];
        }
    }
    return nullptr;
}

Mecanico** listarMecanicos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numMecanicos;
    if (s->numMecanicos == 0) {
        return nullptr;
    }

    Mecanico** resultado = new Mecanico*[s->numMecanicos];
    for (int i = 0; i < s->numMecanicos; i++) {
        resultado[i] = &s->mecanicos[i];
    }

    return resultado;
}