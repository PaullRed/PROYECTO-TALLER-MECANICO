#ifndef REPUESTO_H
#define REPUESTO_H

#include "Estructuras.h"

Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos);
Repuesto* buscarRepuestoPorID(SistemaTaller* s, int id);
Repuesto* buscarRepuestoPorCodigo(SistemaTaller* s, const char* codigo);
Repuesto** buscarRepuestosPorNombre(SistemaTaller* s, const char* subcadena, int* cantidad);
bool registrarEntradaMercancia(SistemaTaller* s, int idRepuesto, int cantidad);
Repuesto** listarRepuestos(SistemaTaller* s, int* cantidad);
bool cambiarPrecioRepuesto(SistemaTaller* s, int idRepuesto, float nuevoPrecio);
bool eliminarRepuesto(SistemaTaller* s, int id);

#endif