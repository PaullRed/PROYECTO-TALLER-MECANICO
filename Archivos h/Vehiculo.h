#ifndef VEHICULO_H
#define VEHICULO_H

#include "Estructura.h"

Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos);
Vehiculo* buscarVehiculoPorID(SistemaTaller* s, int id);
Vehiculo* buscarVehiculoPorPlaca(SistemaTaller* s, const char* placa);
Vehiculo** buscarVehiculosPorPropietario(SistemaTaller* s, const char* subcadena, int* cantidad);
Vehiculo** listarVehiculos(SistemaTaller* s, int* cantidad);
bool actualizarVehiculo(SistemaTaller* s, int id, Vehiculo datos);
bool eliminarVehiculo(SistemaTaller* s, int id);

#endif