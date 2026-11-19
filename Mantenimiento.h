#ifndef MANTENIMIENTO_H
#define MANTENIMIENTO_H

#include <string>

void verificarIntegridad(const std::string& rutaVehiculos,
                         const std::string& rutaMecanicos,
                         const std::string& rutaRepuestos,
                         const std::string& rutaOrdenes);

bool crearBackup(const std::string& rutaTaller,
                 const std::string& rutaVehiculos,
                 const std::string& rutaMecanicos,
                 const std::string& rutaRepuestos,
                 const std::string& rutaOrdenes,
                 std::string& rutaCarpetaCreada);

#endif