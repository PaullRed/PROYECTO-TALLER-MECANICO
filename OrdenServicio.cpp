#include "OrdenServicio.h"
#include "Persistencia.h"
#include "Vehiculo.h"
#include "Mecanico.h"
#include "Repuesto.h"
#include <fstream>
#include <cstring>
#include <ctime>

namespace {
bool buscarPosicionOrdenPorID(std::fstream& archivo, const ArchivoHeader& header, int idOrden, long& posEncontrada, OrdenServicio& ordenEncontrada) {
    if (idOrden <= 0 || idOrden >= header.proximoID || (idOrden - 1) >= header.cantidadRegistros) {
        return false;
    }

    long posDirecta = calcularPosicionRegistro(idOrden - 1, sizeof(OrdenServicio));
    archivo.seekg(posDirecta, std::ios::beg);
    OrdenServicio oTemp;
    archivo.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
    if (!archivo.good() && !archivo.eof()) {
        return false;
    }

    if (oTemp.id == idOrden) {
        posEncontrada = posDirecta;
        ordenEncontrada = oTemp;
        return true;
    }

    long pos = sizeof(ArchivoHeader);
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(pos, std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
        if (oTemp.id == idOrden) {
            posEncontrada = pos;
            ordenEncontrada = oTemp;
            return true;
        }
        pos += sizeof(OrdenServicio);
    }

    return false;
}
}

bool abrirOrdenServicio(const std::string& rutaOrdenes, const std::string& rutaVehiculos, const std::string& rutaMecanicos, int idVehiculo, int idMecanico, int kilometrajeIngreso, const char* descripcionFalla, const char* fechaActual, int& idOrdenAsignada) {
    Vehiculo v;
    if (!obtenerVehiculoPorID(rutaVehiculos, idVehiculo, v) || v.eliminado) {
        return false;
    }

    Mecanico m;
    if (!obtenerMecanicoPorID(rutaMecanicos, idMecanico, m) || m.eliminado) {
        return false;
    }

    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    std::ifstream archivoLectura(rutaOrdenes, std::ios::binary);
    if (!archivoLectura.is_open()) {
        return false;
    }

    OrdenServicio oTemp;
    long pos = sizeof(ArchivoHeader);
    int ordenesAbiertasMecanico = 0;
    for (int i = 0; i < headerOrdenes.cantidadRegistros; i++) {
        archivoLectura.seekg(pos, std::ios::beg);
        archivoLectura.read(reinterpret_cast<char*>(&oTemp), sizeof(OrdenServicio));
        if (strcmp(oTemp.estado, "ABIERTA") == 0) {
            if (oTemp.idVehiculo == idVehiculo) {
                archivoLectura.close();
                return false;
            }
            if (oTemp.idMecanico == idMecanico) {
                ordenesAbiertasMecanico++;
            }
        }
        pos += sizeof(OrdenServicio);
    }
    archivoLectura.close();

    if (ordenesAbiertasMecanico >= 3) {
        return false;
    }

    if (kilometrajeIngreso < v.kilometraje) {
        return false;
    }

    v.kilometraje = kilometrajeIngreso;
    if (!actualizarVehiculo(rutaVehiculos, idVehiculo, v)) {
        return false;
    }

    OrdenServicio nuevaOrden;
    nuevaOrden.id = headerOrdenes.proximoID;
    nuevaOrden.idVehiculo = idVehiculo;
    nuevaOrden.idMecanico = idMecanico;
    strcpy(nuevaOrden.fechaApertura, fechaActual);
    strcpy(nuevaOrden.fechaCierre, "");
    strcpy(nuevaOrden.estado, "ABIERTA");
    strcpy(nuevaOrden.descripcionFalla, descripcionFalla);
    nuevaOrden.kilometrajeIngreso = kilometrajeIngreso;
    nuevaOrden.manoObra = 0.0f;
    nuevaOrden.numDetalles = 0;
    nuevaOrden.eliminado = false;
    nuevaOrden.fechaCreacion = std::time(nullptr);
    nuevaOrden.fechaUltimaModificacion = nuevaOrden.fechaCreacion;

    std::ofstream archivoEscritura(rutaOrdenes, std::ios::binary | std::ios::app);
    if (!archivoEscritura.is_open()) {
        return false;
    }
    archivoEscritura.write(reinterpret_cast<const char*>(&nuevaOrden), sizeof(OrdenServicio));
    if (!archivoEscritura.good()) {
        archivoEscritura.close();
        return false;
    }
    archivoEscritura.close();

    headerOrdenes.cantidadRegistros++;
    headerOrdenes.registrosActivos++;
    headerOrdenes.proximoID++;
    if (!actualizarHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    idOrdenAsignada = nuevaOrden.id;
    return true;
}

bool agregarRepuestoAOrden(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden, int idRepuesto, int cantidad) {
    if (cantidad <= 0) {
        return false;
    }

    Repuesto r;
    if (!obtenerRepuestoPorID(rutaRepuestos, idRepuesto, r) || r.eliminado || r.stock < cantidad) {
        return false;
    }

    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    std::fstream archivo(rutaOrdenes, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    OrdenServicio orden;
    long pos = 0;
    if (!buscarPosicionOrdenPorID(archivo, headerOrdenes, idOrden, pos, orden) || strcmp(orden.estado, "ABIERTA") != 0) {
        archivo.close();
        return false;
    }

    int indiceDetalleExistente = -1;
    for (int i = 0; i < orden.numDetalles; i++) {
        if (orden.detalles[i].idRepuesto == idRepuesto) {
            indiceDetalleExistente = i;
            break;
        }
    }

    if (indiceDetalleExistente != -1) {
        orden.detalles[indiceDetalleExistente].cantidad += cantidad;
    } else {
        if (orden.numDetalles >= OrdenServicio::MAX_DETALLES) {
            archivo.close();
            return false;
        }
        orden.detalles[orden.numDetalles].idRepuesto = idRepuesto;
        orden.detalles[orden.numDetalles].cantidad = cantidad;
        orden.detalles[orden.numDetalles].precioUnitario = r.precio;
        orden.numDetalles++;
    }

    orden.fechaUltimaModificacion = std::time(nullptr);

    r.stock -= cantidad;
    r.fechaUltimaModificacion = std::time(nullptr);

    std::fstream archivoRep(rutaRepuestos, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivoRep.is_open()) {
        archivo.close();
        return false;
    }

    ArchivoHeader headerRep;
    if (!leerHeader(rutaRepuestos, headerRep)) {
        archivoRep.close();
        archivo.close();
        return false;
    }

    Repuesto rTemp;
    long posRep = calcularPosicionRegistro(idRepuesto - 1, sizeof(Repuesto));
    bool repActualizado = false;
    if (idRepuesto > 0 && (idRepuesto - 1) < headerRep.cantidadRegistros) {
        archivoRep.seekg(posRep, std::ios::beg);
        archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (rTemp.id == idRepuesto) {
            archivoRep.seekp(posRep, std::ios::beg);
            archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
            if (!archivoRep.good()) {
                archivoRep.close();
                archivo.close();
                return false;
            }
            repActualizado = true;
        }
    }
    if (!repActualizado) {
        posRep = sizeof(ArchivoHeader);
        for (int i = 0; i < headerRep.cantidadRegistros; i++) {
            archivoRep.seekg(posRep, std::ios::beg);
            archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
            if (rTemp.id == idRepuesto) {
                archivoRep.seekp(posRep, std::ios::beg);
                archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
                if (!archivoRep.good()) {
                    archivoRep.close();
                    archivo.close();
                    return false;
                }
                repActualizado = true;
                break;
            }
            posRep += sizeof(Repuesto);
        }
    }
    archivoRep.close();

    if (!repActualizado) {
        archivo.close();
        return false;
    }

    archivo.seekp(pos, std::ios::beg);
    archivo.write(reinterpret_cast<const char*>(&orden), sizeof(OrdenServicio));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }
    archivo.close();

    return true;
}

bool quitarRepuestoDeOrden(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden, int idRepuesto) {
    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    std::fstream archivo(rutaOrdenes, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    OrdenServicio orden;
    long pos = 0;
    if (!buscarPosicionOrdenPorID(archivo, headerOrdenes, idOrden, pos, orden) || strcmp(orden.estado, "ABIERTA") != 0) {
        archivo.close();
        return false;
    }

    int indiceDetalle = -1;
    for (int i = 0; i < orden.numDetalles; i++) {
        if (orden.detalles[i].idRepuesto == idRepuesto) {
            indiceDetalle = i;
            break;
        }
    }

    if (indiceDetalle == -1) {
        archivo.close();
        return false;
    }

    int cantidadDevuelta = orden.detalles[indiceDetalle].cantidad;

    for (int i = indiceDetalle; i < orden.numDetalles - 1; i++) {
        orden.detalles[i] = orden.detalles[i + 1];
    }
    orden.numDetalles--;
    orden.fechaUltimaModificacion = std::time(nullptr);

    Repuesto r;
    if (!obtenerRepuestoPorID(rutaRepuestos, idRepuesto, r)) {
        archivo.close();
        return false;
    }
    r.stock += cantidadDevuelta;
    r.fechaUltimaModificacion = std::time(nullptr);

    std::fstream archivoRep(rutaRepuestos, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivoRep.is_open()) {
        archivo.close();
        return false;
    }

    ArchivoHeader headerRep;
    if (!leerHeader(rutaRepuestos, headerRep)) {
        archivoRep.close();
        archivo.close();
        return false;
    }

    Repuesto rTemp;
    long posRep = calcularPosicionRegistro(idRepuesto - 1, sizeof(Repuesto));
    bool repActualizado = false;
    if (idRepuesto > 0 && (idRepuesto - 1) < headerRep.cantidadRegistros) {
        archivoRep.seekg(posRep, std::ios::beg);
        archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
        if (rTemp.id == idRepuesto) {
            archivoRep.seekp(posRep, std::ios::beg);
            archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
            if (!archivoRep.good()) {
                archivoRep.close();
                archivo.close();
                return false;
            }
            repActualizado = true;
        }
    }
    if (!repActualizado) {
        posRep = sizeof(ArchivoHeader);
        for (int i = 0; i < headerRep.cantidadRegistros; i++) {
            archivoRep.seekg(posRep, std::ios::beg);
            archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
            if (rTemp.id == idRepuesto) {
                archivoRep.seekp(posRep, std::ios::beg);
                archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
                if (!archivoRep.good()) {
                    archivoRep.close();
                    archivo.close();
                    return false;
                }
                repActualizado = true;
                break;
            }
            posRep += sizeof(Repuesto);
        }
    }
    archivoRep.close();

    if (!repActualizado) {
        archivo.close();
        return false;
    }

    archivo.seekp(pos, std::ios::beg);
    archivo.write(reinterpret_cast<const char*>(&orden), sizeof(OrdenServicio));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }
    archivo.close();

    return true;
}

bool cerrarOrdenServicio(const std::string& rutaOrdenes, int idOrden, float manoObra, const char* fechaCierre) {
    if (manoObra < 0) {
        return false;
    }

    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    std::fstream archivo(rutaOrdenes, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    OrdenServicio orden;
    long pos = 0;
    if (!buscarPosicionOrdenPorID(archivo, headerOrdenes, idOrden, pos, orden) || strcmp(orden.estado, "ABIERTA") != 0) {
        archivo.close();
        return false;
    }

    if (orden.numDetalles == 0 && manoObra <= 0) {
        archivo.close();
        return false;
    }

    orden.manoObra = manoObra;
    strcpy(orden.estado, "CERRADA");
    strcpy(orden.fechaCierre, fechaCierre);
    orden.fechaUltimaModificacion = std::time(nullptr);

    archivo.seekp(pos, std::ios::beg);
    archivo.write(reinterpret_cast<const char*>(&orden), sizeof(OrdenServicio));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }
    archivo.close();

    return true;
}

bool anularOrdenServicio(const std::string& rutaOrdenes, const std::string& rutaRepuestos, int idOrden) {
    ArchivoHeader headerOrdenes;
    if (!leerHeader(rutaOrdenes, headerOrdenes)) {
        return false;
    }

    std::fstream archivo(rutaOrdenes, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo.is_open()) {
        return false;
    }

    OrdenServicio orden;
    long pos = 0;
    if (!buscarPosicionOrdenPorID(archivo, headerOrdenes, idOrden, pos, orden) || strcmp(orden.estado, "ANULADA") == 0) {
        archivo.close();
        return false;
    }

    for (int i = 0; i < orden.numDetalles; i++) {
        Repuesto r;
        if (!obtenerRepuestoPorID(rutaRepuestos, orden.detalles[i].idRepuesto, r)) {
            archivo.close();
            return false;
        }
        r.stock += orden.detalles[i].cantidad;
        r.fechaUltimaModificacion = std::time(nullptr);

        std::fstream archivoRep(rutaRepuestos, std::ios::binary | std::ios::in | std::ios::out);
        if (!archivoRep.is_open()) {
            archivo.close();
            return false;
        }

        ArchivoHeader headerRep;
        if (!leerHeader(rutaRepuestos, headerRep)) {
            archivoRep.close();
            archivo.close();
            return false;
        }

        Repuesto rTemp;
        long posRep = calcularPosicionRegistro(orden.detalles[i].idRepuesto - 1, sizeof(Repuesto));
        bool repActualizado = false;
        if (orden.detalles[i].idRepuesto > 0 && (orden.detalles[i].idRepuesto - 1) < headerRep.cantidadRegistros) {
            archivoRep.seekg(posRep, std::ios::beg);
            archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
            if (rTemp.id == orden.detalles[i].idRepuesto) {
                archivoRep.seekp(posRep, std::ios::beg);
                archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
                if (!archivoRep.good()) {
                    archivoRep.close();
                    archivo.close();
                    return false;
                }
                repActualizado = true;
            }
        }
        if (!repActualizado) {
            posRep = sizeof(ArchivoHeader);
            for (int j = 0; j < headerRep.cantidadRegistros; j++) {
                archivoRep.seekg(posRep, std::ios::beg);
                archivoRep.read(reinterpret_cast<char*>(&rTemp), sizeof(Repuesto));
                if (rTemp.id == orden.detalles[i].idRepuesto) {
                    archivoRep.seekp(posRep, std::ios::beg);
                    archivoRep.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));
                    if (!archivoRep.good()) {
                        archivoRep.close();
                        archivo.close();
                        return false;
                    }
                    repActualizado = true;
                    break;
                }
                posRep += sizeof(Repuesto);
            }
        }
        archivoRep.close();

        if (!repActualizado) {
            archivo.close();
            return false;
        }
    }

    strcpy(orden.estado, "ANULADA");
    orden.fechaUltimaModificacion = std::time(nullptr);

    archivo.seekp(pos, std::ios::beg);
    archivo.write(reinterpret_cast<const char*>(&orden), sizeof(OrdenServicio));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }
    archivo.close();

    return true;
}

float calcularTotalOrdenServicio(const OrdenServicio& orden) {
    float total = orden.manoObra;
    for (int i = 0; i < orden.numDetalles; i++) {
        total += orden.detalles[i].cantidad * orden.detalles[i].precioUnitario;
    }
    return total;
}

bool obtenerOrdenPorID(const std::string& rutaOrdenes, int id, OrdenServicio& ordenEncontrada) {
    ArchivoHeader header;
    if (!leerHeader(rutaOrdenes, header)) {
        return false;
    }

    if (id <= 0 || id >= header.proximoID || (id - 1) >= header.cantidadRegistros) {
        return false;
    }

    std::fstream archivo(rutaOrdenes, std::ios::binary | std::ios::in);
    if (!archivo.is_open()) {
        return false;
    }

    long pos = 0;
    bool encontrada = buscarPosicionOrdenPorID(archivo, header, id, pos, ordenEncontrada);
    archivo.close();
    return encontrada;
}