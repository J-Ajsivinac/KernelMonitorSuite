#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/thermal.h> 

/**
 * Syscall para obtener temperatura de la CPU en grados Celsius
 * @return Temperatura en grados Celsius, o error, al usar maquina virtual se toma los ultimos 4 numeros del carnet para el calculo
 */
SYSCALL_DEFINE0(get_cpu_temp)
{
    struct thermal_zone_device *tzd;
    int temp, ret;
    
    // Intentar obtener el dispositivo de zona térmica principal
    tzd = thermal_zone_get_zone_by_name("cpu-thermal");
    if (IS_ERR(tzd)) {
        // Intentar con otro nombre común
        tzd = thermal_zone_get_zone_by_name("cpu_thermal");
        if (IS_ERR(tzd)) {
            // Si no se encuentra, devolver el carnet
            //return -ENODEV; // No se encontró dispositivo de temperatura
            int carnet = 135;
            return carnet;
        }
    }
    
    ret = thermal_zone_get_temp(tzd, &temp);
    if (ret){
        return ret;
    }
    // La temperatura se devuelve en milígrados, convertir a grados Celsius
    return temp / 1000;
}
