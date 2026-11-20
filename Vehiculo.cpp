#include "Vehiculo.h"
#include "Persistencia.h"
#include <fstream>
#include <cstring>
#include <cctype>
#include <ctime>

namespace {
std::string aMinusculasVehiculo(const std::string& texto) {
    std::string copia = texto;
    for (size_t i = 0; i < copia.size(); i++) {
        copia[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(copia[i])));
    }
    return copia;
}
}

bool registrarVehiculo(const std::string& rutaArchivo, Vehiculo datosVehiculo, int& idAsignado) {
    if (datosVehiculo.anio < 1950 || datosVehiculo.anio > 2027 || datosVehiculo.kilometraje < 0) {
        return false;
    }

    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::ifstream archivoLectura(rutaArchivo, std::ios::binary);
    if (!archivoLectura.is_open()) {
        return false;
    }

    Vehiculo vTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivoLectura.seekg(pos, std::ios::beg);
        archivoLectura.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
        if (!vTemp.eliminado && strcmp(vTemp.placa, datosVehiculo.placa) == 0) {
            archivoLectura.close();
            return false;
        }
        pos += sizeof(Vehiculo);
    }
    archivoLectura.close();

    datosVehiculo.id = header.proximoID;
    datosVehiculo.eliminado = false;
    datosVehiculo.fechaCreacion = std::time(nullptr);
    datosVehiculo.fechaUltimaModificacion = datosVehiculo.fechaCreacion;

    std::ofstream archivoEscritura(rutaArchivo, std::ios::binary | std::ios::app);
    if (!archivoEscritura.is_open()) {
        return false;
    }
    archivoEscritura.write(reinterpret_cast<const char*>(&datosVehiculo), sizeof(Vehiculo));
    if (!archivoEscritura.good()) {
        archivoEscritura.close();
        return false;
    }
    archivoEscritura.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;

    if (!actualizarHeader(rutaArchivo, header)) {
        return false;
    }

    idAsignado = datosVehiculo.id;
    return true;
}

bool obtenerVehiculoPorID(const std::string& rutaArchivo, int id, Vehiculo& vehiculoEncontrado) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    if (id <= 0 || id >= header.proximoID || (id - 1) >= header.cantidadRegistros) {
        return false;
    }

    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    long pos = calcularPosicionRegistro(id - 1, sizeof(Vehiculo));
    archivo.seekg(pos, std::ios::beg);
    Vehiculo vTemp;
    archivo.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
    archivo.close();

    if (!archivo.good() && !archivo.eof()) {
        return false;
    }

    if (vTemp.id != id) {
        return false;
    }

    vehiculoEncontrado = vTemp;
    return true;
}

bool obtenerVehiculoPorPlaca(const std::string& rutaArchivo, const std::string& placa, Vehiculo& vehiculoEncontrado) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    Vehiculo vTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
        if (!vTemp.eliminado && strcmp(vTemp.placa, placa.c_str()) == 0) {
            vehiculoEncontrado = vTemp;
            archivo.close();
            return true;
        }
        pos += sizeof(Vehiculo);
    }

    archivo.close();
    return false;
}

bool buscarVehiculosPorPropietarioParcial(const std::string& rutaArchivo,
                                         const std::string& fragmentoNombre,
                                         Vehiculo resultados[],
                                         int maxResultados,
                                         int& cantidadEncontrados,
                                         bool& hayMas) {
    cantidadEncontrados = 0;
    hayMas = false;

    if (maxResultados <= 0 || fragmentoNombre.empty()) {
        return false;
    }

    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    std::string filtro = aMinusculasVehiculo(fragmentoNombre);
    Vehiculo vTemp;
    long pos = sizeof(ArchivoHeader);

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
        if (!archivo.good() && !archivo.eof()) {
            archivo.close();
            return false;
        }

        if (!vTemp.eliminado) {
            std::string propMinuscula = aMinusculasVehiculo(std::string(vTemp.nombrePropietario));
            if (propMinuscula.find(filtro) != std::string::npos) {
                if (cantidadEncontrados < maxResultados) {
                    resultados[cantidadEncontrados] = vTemp;
                    cantidadEncontrados++;
                } else {
                    hayMas = true;
                }
            }
        }
        pos += sizeof(Vehiculo);
    }

    archivo.close();
    return true;
}

bool actualizarVehiculo(const std::string& rutaArchivo, int id, const Vehiculo& nuevosDatos) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::fstream archivo(rutaArchivo, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    Vehiculo vTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
        if (vTemp.id == id && !vTemp.eliminado) {
            if (nuevosDatos.anio < 1950 || nuevosDatos.anio > 2027) {
                archivo.close();
                return false;
            }
            strcpy(vTemp.marca, nuevosDatos.marca);
            strcpy(vTemp.modelo, nuevosDatos.modelo);
            vTemp.anio = nuevosDatos.anio;
            strcpy(vTemp.nombrePropietario, nuevosDatos.nombrePropietario);
            strcpy(vTemp.cedulaPropietario, nuevosDatos.cedulaPropietario);
            strcpy(vTemp.telefonoPropietario, nuevosDatos.telefonoPropietario);
            vTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&vTemp), sizeof(Vehiculo));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Vehiculo);
    }

    archivo.close();
    return encontrado;
}

bool eliminarVehiculoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes) {
    ArchivoHeader headerOrdenes;
    if (leerHeader(rutaOrdenes, headerOrdenes)) {
        std::ifstream archOrd(rutaOrdenes, std::ios::binary);
        if (archOrd.is_open()) {
            OrdenServicio oTemp;
            long posOrd = sizeof(ArchivoHeader);
            for (int i = 0; i < headerOrdenes.cantidadRegistros; i++) {
                archOrd.seekg(posOrd, std::ios::beg);
                archOrd.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
                if (oTemp.idVehiculo == id) {
                    archOrd.close();
                    return false;
                }
                posOrd += sizeof(OrdenServicio);
            }
            archOrd.close();
        }
    }

    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::fstream archivo(rutaArchivo, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    Vehiculo vTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&vTemp), sizeof(Vehiculo));
        if (vTemp.id == id && !vTemp.eliminado) {
            vTemp.eliminado = true;
            vTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&vTemp), sizeof(Vehiculo));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Vehiculo);
    }

    archivo.close();

    if (encontrado) {
        header.registrosActivos--;
        if (!actualizarHeader(rutaArchivo, header)) {
            return false;
        }
        return true;
    }

    return false;
}

bool contarVehiculosActivos(const std::string& rutaArchivo, int& cantidadActivos) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }
    cantidadActivos = header.registrosActivos;
    return true;
}