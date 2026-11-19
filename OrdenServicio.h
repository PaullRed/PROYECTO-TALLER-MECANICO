#ifndef ORDEN_SERVICIO_H
#define ORDEN_SERVICIO_H

#include "Estructura.h"
#include <string>

bool abrirOrdenServicio(const std::string& rutaOrdenes, const std::string& rutaVehiculos, const std::string& rutaMecanicos, int idVehiculo, int idMecanico, int kilometrajeIngreso, const char* descripcionFalla, const char* fechaActual, int& idOrdenAsignada);
bool agregarRepuestoAOrden(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden, int idRepuesto, int cantidad);
bool quitarRepuestoDeOrden(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden, int idRepuesto);
bool cerrarOrdenServicio(const std::string& rutaOrdenes, int idOrden, float manoObra, const char* fechaCierre);
bool anularOrdenServicio(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden);
float calcularTotalOrdenServicio(const OrdenServicio& orden);
bool obtenerOrdenPorID(const std::string& rutaOrdenes, int id, OrdenServicio& ordenEncontrada);

#endif