#ifndef ORDEN_SERVICIO_H
#define ORDEN_SERVICIO_H

#include "Estructura.h"

OrdenServicio* abrirOrden(SistemaTaller* s, int idVehiculo, int idMecanico, int kilometrajeIngreso, const char* descripcionFalla, const char* fechaActual);
bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad);
bool quitarRepuestoDeOrden(SistemaTaller* s, int idOrden, int idRepuesto);
bool cerrarOrden(SistemaTaller* s, int idOrden, float manoObra, const char* fechaCierre);
bool anularOrden(SistemaTaller* s, int idOrden);
float calcularTotalOrden(OrdenServicio orden);
OrdenServicio* buscarOrdenPorID(SistemaTaller* s, int id);
OrdenServicio** listarOrdenesPorEstado(SistemaTaller* s, const char* estado, int* cantidad);
OrdenServicio** listarHistorialVehiculo(SistemaTaller* s, int idVehiculo, int* cantidad);

#endif