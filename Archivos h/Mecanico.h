#ifndef MECANICO_H
#define MECANICO_H

#include "Estructuras.h"

Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos);
Mecanico* buscarMecanicoPorID(SistemaTaller* s, int id);
Mecanico* buscarMecanicoPorCedula(SistemaTaller* s, const char* cedula);
Mecanico** listarMecanicos(SistemaTaller* s, int* cantidad);

#endif