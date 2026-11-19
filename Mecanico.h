#ifndef MECANICO_H
#define MECANICO_H

#include "Estructura.h"
#include <string>

bool registrarMecanico(const std::string& rutaArchivo, Mecanico datosMecanico, int& idAsignado);
bool obtenerMecanicoPorID(const std::string& rutaArchivo, int id, Mecanico& mecanicoEncontrado);
bool obtenerMecanicoPorCedula(const std::string& rutaArchivo, const std::string& cedula, Mecanico& mecanicoEncontrado);
bool darDeBajaMecanicoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes = "datos/ordenes.bin");
bool esEspecialidadValidaMecanico(const char* esp);

#endif