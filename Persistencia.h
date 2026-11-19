#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <fstream>
#include <string>
#include "Estructura.h"

bool inicializarArchivoEntidad(const std::string& ruta);
bool leerHeader(const std::string& ruta, ArchivoHeader& header);
bool actualizarHeader(const std::string& ruta, const ArchivoHeader& header);
long calcularPosicionRegistro(int indiceFisico, size_t tamanoEstructura);

#endif