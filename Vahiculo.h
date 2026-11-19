#ifndef VEHICULO_H
#define VEHICULO_H

#include "Estructura.h"
#include <string>

bool registrarVehiculo(const std::string& rutaArchivo, Vehiculo datosVehiculo, int& idAsignado);
bool obtenerVehiculoPorID(const std::string& rutaArchivo, int id, Vehiculo& vehiculoEncontrado);
bool obtenerVehiculoPorPlaca(const std::string& rutaArchivo, const std::string& placa, Vehiculo& vehiculoEncontrado);
bool buscarVehiculosPorPropietarioParcial(const std::string& rutaArchivo,
                                         const std::string& fragmentoNombre,
                                         Vehiculo resultados[],
                                         int maxResultados,
                                         int& cantidadEncontrados,
                                         bool& hayMas);
bool actualizarVehiculo(const std::string& rutaArchivo, int id, const Vehiculo& nuevosDatos);
bool eliminarVehiculoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes = "datos/ordenes.bin");
bool contarVehiculosActivos(const std::string& rutaArchivo, int& cantidadActivos);

#endif