#include "Repuesto.h"
#include "Persistencia.h"
#include <fstream>
#include <cstring>
#include <cctype>
#include <ctime>

namespace {
std::string aMinusculasRepuesto(const std::string& texto) {
    std::string copia = texto;
    for (size_t i = 0; i < copia.size(); i++) {
        copia[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(copia[i])));
    }
    return copia;
}
}

bool registrarRepuesto(const std::string& rutaArchivo, Repuesto datosRepuesto, int& idAsignado) {
    if (datosRepuesto.precio <= 0 || datosRepuesto.stock < 0 || datosRepuesto.stockMinimo < 0) {
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

    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivoLectura.seekg(pos, std::ios::beg);
        archivoLectura.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (!rTemp.eliminado && strcmp(rTemp.codigo, datosRepuesto.codigo) == 0) {
            archivoLectura.close();
            return false;
        }
        pos += sizeof(Repuesto);
    }
    archivoLectura.close();

    datosRepuesto.id = header.proximoID;
    datosRepuesto.eliminado = false;
    datosRepuesto.fechaCreacion = std::time(nullptr);
    datosRepuesto.fechaUltimaModificacion = datosRepuesto.fechaCreacion;

    std::ofstream archivoEscritura(rutaArchivo, std::ios::binary | std::ios::app);
    if (!archivoEscritura.is_open()) {
        return false;
    }
    archivoEscritura.write(reinterpret_cast<const char*>(&datosRepuesto), sizeof(Repuesto));
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

    idAsignado = datosRepuesto.id;
    return true;
}

bool obtenerRepuestoPorID(const std::string& rutaArchivo, int id, Repuesto& repuestoEncontrado) {
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

    long pos = calcularPosicionRegistro(id - 1, sizeof(Repuesto));
    archivo.seekg(pos, std::ios::beg);
    Repuesto rTemp;
    archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
    archivo.close();

    if (!archivo.good() && !archivo.eof()) {
        return false;
    }

    if (rTemp.id != id) {
        return false;
    }

    repuestoEncontrado = rTemp;
    return true;
}

bool obtenerRepuestoPorCodigo(const std::string& rutaArchivo, const std::string& codigo, Repuesto& repuestoEncontrado) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (!rTemp.eliminado && strcmp(rTemp.codigo, codigo.c_str()) == 0) {
            repuestoEncontrado = rTemp;
            archivo.close();
            return true;
        }
        pos += sizeof(Repuesto);
    }

    archivo.close();
    return false;
}

bool buscarRepuestosPorNombreParcial(const std::string& rutaArchivo,
                                     const std::string& fragmentoNombre,
                                     Repuesto resultados[],
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

    std::string filtro = aMinusculasRepuesto(fragmentoNombre);
    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (!archivo.good() && !archivo.eof()) {
            archivo.close();
            return false;
        }

        if (!rTemp.eliminado) {
            std::string nombreMinuscula = aMinusculasRepuesto(std::string(rTemp.nombre));
            if (nombreMinuscula.find(filtro) != std::string::npos) {
                if (cantidadEncontrados < maxResultados) {
                    resultados[cantidadEncontrados] = rTemp;
                    cantidadEncontrados++;
                } else {
                    hayMas = true;
                }
            }
        }
        pos += sizeof(Repuesto);
    }

    archivo.close();
    return true;
}

bool registrarEntradaMercancia(const std::string& rutaArchivo, int idRepuesto, int cantidad) {
    if (cantidad <= 0) {
        return false;
    }

    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::fstream archivo(rutaArchivo, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (rTemp.id == idRepuesto && !rTemp.eliminado) {
            rTemp.stock += cantidad;
            rTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&rTemp), sizeof(Repuesto));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Repuesto);
    }

    archivo.close();
    return encontrado;
}

bool cambiarPrecioRepuesto(const std::string& rutaArchivo, int idRepuesto, float nuevoPrecio) {
    if (nuevoPrecio <= 0) {
        return false;
    }

    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::fstream archivo(rutaArchivo, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (rTemp.id == idRepuesto && !rTemp.eliminado) {
            rTemp.precio = nuevoPrecio;
            rTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&rTemp), sizeof(Repuesto));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Repuesto);
    }

    archivo.close();
    return encontrado;
}

bool eliminarRepuestoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes) {
    ArchivoHeader headerOrdenes;
    if (leerHeader(rutaOrdenes, headerOrdenes)) {
        std::ifstream archOrd(rutaOrdenes, std::ios::binary);
        if (archOrd.is_open()) {
            OrdenServicio oTemp;
            long posOrd = sizeof(ArchivoHeader);
            for (int i = 0; i < headerOrdenes.cantidadRegistros; i++) {
                archOrd.seekg(posOrd, std::ios::beg);
                archOrd.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
                for (int d = 0; d < oTemp.numDetalles; d++) {
                    if (oTemp.detalles[d].idRepuesto == id) {
                        archOrd.close();
                        return false;
                    }
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

    Repuesto rTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (rTemp.id == id && !rTemp.eliminado) {
            rTemp.eliminado = true;
            rTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&rTemp), sizeof(Repuesto));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Repuesto);
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