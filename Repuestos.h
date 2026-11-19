#ifndef REPUESTO_H
#define REPUESTO_H

#include "Estructura.h"
#include <string>

bool registrarRepuesto(const std::string& rutaArchivo, Repuesto datosRepuesto, int& idAsignado);
bool obtenerRepuestoPorID(const std::string& rutaArchivo, int id, Repuesto& repuestoEncontrado);
bool obtenerRepuestoPorCodigo(const std::string& rutaArchivo, const std::string& codigo, Repuesto& repuestoEncontrado);
bool buscarRepuestosPorNombreParcial(const std::string& rutaArchivo,
                                     const std::string& fragmentoNombre,
                                     Repuesto resultados[],
                                     int maxResultados,
                                     int& cantidadEncontrados,
                                     bool& hayMas);
bool registrarEntradaMercancia(const std::string& rutaArchivo, int idRepuesto, int cantidad);
bool cambiarPrecioRepuesto(const std::string& rutaArchivo, int idRepuesto, float nuevoPrecio);
bool eliminarRepuestoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes = "datos/ordenes.bin");

#endif