#include "Persistencia.h"

bool inicializarArchivoEntidad(const std::string& ruta) {
    std::ifstream archivoPrueba(ruta, std::ios::binary);
    if (!archivoPrueba.is_open()) {
        std::ofstream archivoNuevo(ruta, std::ios::binary);
        if (!archivoNuevo.is_open()) {
            return false;
        }
        ArchivoHeader headerInicial;
        headerInicial.cantidadRegistros = 0;
        headerInicial.proximoID = 1;
        headerInicial.registrosActivos = 0;
        headerInicial.version = 1;
        archivoNuevo.write(reinterpret_cast<const char*>(&headerInicial), sizeof(ArchivoHeader));
        archivoNuevo.close();
    } else {
        archivoPrueba.close();
    }
    return true;
}

bool leerHeader(const std::string& ruta, ArchivoHeader& header) {
    std::ifstream archivo(ruta, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool actualizarHeader(const std::string& ruta, const ArchivoHeader& header) {
    std::fstream archivo(ruta, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.seekp(0, std::ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

long calcularPosicionRegistro(int indiceFisico, size_t tamanoEstructura) {
    return sizeof(ArchivoHeader) + (static_cast<long>(indiceFisico) * static_cast<long>(tamanoEstructura));
}