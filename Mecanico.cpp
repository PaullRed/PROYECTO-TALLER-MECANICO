#include "Mecanico.h"
#include "Persistencia.h"
#include <fstream>
#include <cstring>
#include <ctime>

bool esEspecialidadValidaMecanico(const char* esp) {
    return strcmp(esp, "MOTOR") == 0 ||
           strcmp(esp, "FRENOS") == 0 ||
           strcmp(esp, "ELECTRICIDAD") == 0 ||
           strcmp(esp, "SUSPENSION") == 0 ||
           strcmp(esp, "GENERAL") == 0;
}

bool registrarMecanico(const std::string& rutaArchivo, Mecanico datosMecanico, int& idAsignado) {
    if (!esEspecialidadValidaMecanico(datosMecanico.especialidad)) {
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

    Mecanico mTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivoLectura.seekg(pos, std::ios::beg);
        archivoLectura.read(reinterpret_cast<char*>(&mTemp), sizeof(Mecanico));
        if (!mTemp.eliminado && strcmp(mTemp.cedula, datosMecanico.cedula) == 0) {
            archivoLectura.close();
            return false;
        }
        pos += sizeof(Mecanico);
    }
    archivoLectura.close();

    datosMecanico.id = header.proximoID;
    datosMecanico.eliminado = false;
    datosMecanico.fechaCreacion = std::time(nullptr);
    datosMecanico.fechaUltimaModificacion = datosMecanico.fechaCreacion;

    std::ofstream archivoEscritura(rutaArchivo, std::ios::binary | std::ios::app);
    if (!archivoEscritura.is_open()) {
        return false;
    }
    archivoEscritura.write(reinterpret_cast<const char*>(&datosMecanico), sizeof(Mecanico));
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

    idAsignado = datosMecanico.id;
    return true;
}

bool obtenerMecanicoPorID(const std::string& rutaArchivo, int id, Mecanico& mecanicoEncontrado) {
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

    long pos = calcularPosicionRegistro(id - 1, sizeof(Mecanico));
    archivo.seekg(pos, std::ios::beg);
    Mecanico mTemp;
    archivo.read(reinterpret_cast<char*>(&mTemp), sizeof(Mecanico));
    archivo.close();

    if (!archivo.good() && !archivo.eof()) {
        return false;
    }

    if (mTemp.id != id) {
        return false;
    }

    mecanicoEncontrado = mTemp;
    return true;
}

bool obtenerMecanicoPorCedula(const std::string& rutaArchivo, const std::string& cedula, Mecanico& mecanicoEncontrado) {
    ArchivoHeader header;
    if (!leerHeader(rutaArchivo, header)) {
        return false;
    }

    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    Mecanico mTemp;
    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&mTemp), sizeof(Mecanico));
        if (!mTemp.eliminado && strcmp(mTemp.cedula, cedula.c_str()) == 0) {
            mecanicoEncontrado = mTemp;
            archivo.close();
            return true;
        }
        pos += sizeof(Mecanico);
    }

    archivo.close();
    return false;
}

bool darDeBajaMecanicoLogico(const std::string& rutaArchivo, int id, const std::string& rutaOrdenes) {
    ArchivoHeader headerOrdenes;
    if (leerHeader(rutaOrdenes, headerOrdenes)) {
        std::ifstream archOrd(rutaOrdenes, std::ios::binary);
        if (archOrd.is_open()) {
            OrdenServicio oTemp;
            long posOrd = sizeof(ArchivoHeader);
            for (int i = 0; i < headerOrdenes.cantidadRegistros; i++) {
                archOrd.seekg(posOrd, std::ios::beg);
                archOrd.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
                if (oTemp.idMecanico == id && std::strcmp(oTemp.estado, "ABIERTA") == 0) {
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

    Mecanico mTemp;
    long pos = sizeof(ArchivoHeader);
    bool encontrado = false;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&mTemp), sizeof(Mecanico));
        if (mTemp.id == id && !mTemp.eliminado) {
            mTemp.eliminado = true;
            mTemp.fechaUltimaModificacion = std::time(nullptr);

            archivo.seekp(pos, std::ios::beg);
            archivo.write(reinterpret_cast<const char*>(&mTemp), sizeof(Mecanico));
            if (!archivo.good()) {
                archivo.close();
                return false;
            }
            encontrado = true;
            break;
        }
        pos += sizeof(Mecanico);
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