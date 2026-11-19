#ifndef REPORTES_H
#define REPORTES_H

#include <string>

void reporteHistorialVehiculo(const std::string& placa,
                              const std::string& rutaVehiculos,
                              const std::string& rutaMecanicos,
                              const std::string& rutaOrdenes);

void reporteRepuestosMasUtilizados(const std::string& rutaRepuestos,
                                   const std::string& rutaOrdenes);

void reporteFacturacionPorMecanico(const std::string& rutaMecanicos,
                                   const std::string& rutaOrdenes);

#endif