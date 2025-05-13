import { useState, useEffect, useRef } from "react";

export default function ProcessTimeline({ processes = [], currentTimeOverride = null }) {
    const timelineRef = useRef(null);
    const [currentTime, setCurrentTime] = useState(new Date());
    const [lastUpdateTime, setLastUpdateTime] = useState(new Date());
    
    // Función para formatear tiempo actual a formato HH:MM:SS
    const formatCurrentTime = (date) => {
        const hours = date.getHours().toString().padStart(2, '0');
        const minutes = date.getMinutes().toString().padStart(2, '0');
        const seconds = date.getSeconds().toString().padStart(2, '0');
        return `${hours}:${minutes}:${seconds}`;
    };

    // Función para convertir fechas ISO de la API a formato HH:MM:SS
    const formatApiDateToTime = (dateString) => {
        if (!dateString) return null;
        
        // Convertir la cadena a objeto Date
        const date = new Date(dateString);
        
        // Extraer solo la parte de hora
        const hours = date.getHours().toString().padStart(2, '0');
        const minutes = date.getMinutes().toString().padStart(2, '0');
        const seconds = date.getSeconds().toString().padStart(2, '0');
        
        return `${hours}:${minutes}:${seconds}`;
    };

    // Procesar los procesos para tener el formato de hora correcto
    const formattedProcesses = processes.map(process => ({
        ...process,
        startTime: formatApiDateToTime(process.startTime),
        endTime: process.endTime ? formatApiDateToTime(process.endTime) : null
    }));

    // Convertir tiempo en formato "HH:MM:SS" a minutos totales para cálculos
    const timeToMinutes = (timeString) => {
        if (!timeString) return 0;
        const [hours, minutes, seconds] = timeString.split(":").map(Number);
        return hours * 60 + minutes + seconds / 60;
    };

    // Para simular el avance del tiempo más rápido (si no se proporciona currentTimeOverride)
    useEffect(() => {
        if (currentTimeOverride) {
            setCurrentTime(new Date(currentTimeOverride));
            return;
        }
        
        const timeSpeed = 10; // Factor de aceleración (20x más rápido)
        
        const timeInterval = setInterval(() => {
            const now = new Date();
            // Calculamos cuánto tiempo ha pasado desde la última actualización
            const elapsedMs = now.getTime() - lastUpdateTime.getTime();
            // Aplicamos el factor de aceleración
            const acceleratedMs = elapsedMs * timeSpeed;
            
            // Creamos una nueva fecha acelerada
            const acceleratedTime = new Date(currentTime.getTime() + acceleratedMs);
            
            setCurrentTime(acceleratedTime);
            setLastUpdateTime(now);
        }, 1000);

        return () => {
            clearInterval(timeInterval);
        };
    }, [currentTime, lastUpdateTime, currentTimeOverride]);

    // Auto-scroll para mantener visibles los procesos recientes
    useEffect(() => {
        const scrollToRight = () => {
            if (timelineRef.current) {
                // Scroll suave hacia la derecha
                timelineRef.current.scrollLeft = timelineRef.current.scrollWidth;
            }
        };
        
        const scrollInterval = setInterval(scrollToRight, 5000);

        return () => {
            clearInterval(scrollInterval);
        };
    }, []);

    // Encontrar los límites de tiempo para la escala del gráfico
    const calculateTimeRange = () => {
        if (formattedProcesses.length === 0) return { minTime: 0, maxTime: 60, timeRange: 60 };
        
        const minTime = Math.min(...formattedProcesses.map(p => timeToMinutes(p.startTime)));
        
        // Para el tiempo máximo, usamos el tiempo actual para procesos en ejecución
        const currentTimeStr = formatCurrentTime(currentTime);
        const maxTime = Math.max(
            ...formattedProcesses.map(p => p.endTime ? timeToMinutes(p.endTime) : timeToMinutes(currentTimeStr)),
            timeToMinutes(currentTimeStr) // Siempre incluir el tiempo actual
        );
        
        // Añadir margen al final
        return { 
            minTime, 
            maxTime: maxTime + 5, 
            timeRange: (maxTime + 5) - minTime 
        };
    };

    const { minTime, maxTime, timeRange } = calculateTimeRange();

    // Crear marcas de tiempo para el eje X
    const generateTimeMarks = () => {
        const startHour = Math.floor(minTime / 60);
        const endHour = Math.ceil(maxTime / 60);
        const marks = [];

        for (let hour = startHour; hour <= endHour; hour++) {
            for (let minute = 0; minute < 60; minute += 20) {
                if (hour === endHour && minute > (maxTime % 60)) continue;

                const totalMinutes = hour * 60 + minute;
                if (totalMinutes >= minTime && totalMinutes <= maxTime) {
                    const position = ((totalMinutes - minTime) / timeRange) * 100;
                    marks.push({
                        time: `${hour.toString().padStart(2, "0")}:${minute.toString().padStart(2, "0")}`,
                        position: position,
                    });
                }
            }
        }
        return marks;
    };

    const timeMarks = generateTimeMarks();
    
    // Calcular posición del tiempo actual
    const currentTimeMinutes = timeToMinutes(formatCurrentTime(currentTime));
    const currentTimePosition = ((currentTimeMinutes - minTime) / timeRange) * 100;

    // Contador de procesos activos
    const activeProcesses = formattedProcesses.filter(p => p.endTime === null).length;

    return (
        <div className="w-full py-2 px-4 rounded-lg h-[95%]">
            {/* Panel de información */}
            <div className="flex justify-between mb-2 text-sm text-gray-500">
                <div>Tiempo actual: {formatCurrentTime(currentTime)}</div>
                <div>Procesos activos: {activeProcesses}</div>
            </div>
            
            {/* Contenedor principal con scroll horizontal */}
            <div 
                ref={timelineRef}
                className="overflow-x-auto h-[96%]"
                style={{ minWidth: "100%" }}
            >       
                <div className="relative min-w-full h-[90%]" style={{ minWidth: "max(800px, 100%)" }}>
                    {/* Eje de tiempo */}
                    <div className="relative h-5 mb-3">
                        {timeMarks.map((mark, index) => (
                            <div
                                key={index}
                                className="absolute top-0 flex flex-col items-center"
                                style={{ left: `${mark.position}%` }}
                            >
                                <div className="h-2 w-px bg-gray-400"></div>
                                <span className="text-xs text-gray-400 mt-1">
                                    {mark.time}
                                </span>
                            </div>
                        ))}
                        
                        {/* Línea de tiempo actual */}
                        <div 
                            className="absolute top-0 h-3 w-1 bg-red-500"
                            style={{ left: `${currentTimePosition-1}%` }}
                        >
                            <div className="w-3 h-3 rounded-full bg-red-500 -ml-1 -mt-1"></div>
                        </div>
                    </div>

                    {/* Barras de procesos */}
                    <div className="space-y-5 overflow-y-auto px-3 py-2 h-full">
                        {formattedProcesses.map((process) => {
                            const startMinutes = timeToMinutes(process.startTime);
                            // Para procesos en ejecución, usamos tiempo actual
                            const endMinutes = process.endTime 
                                ? timeToMinutes(process.endTime) 
                                : currentTimeMinutes;

                            const startPosition = ((startMinutes - minTime) / timeRange) * 100;
                            const endPosition = ((endMinutes - minTime) / timeRange) * 100;
                            const width = endPosition - startPosition;

                            // Mantener los colores originales
                            const processColor = process.endTime
                                ? "bg-[#2d4a48] border-2 border-[#4e8f7b]"
                                : "bg-[#333d53] border-2 border-[#838dc2]";

                            // Animación para procesos en ejecución
                            const animationClass = process.endTime === null ? "transition-all duration-1000" : "";

                            return (
                                <div key={process.id} className="relative h-9">
                                    {/* Línea horizontal */}
                                    <div className="absolute w-full border-1 border-gray-400 border-dashed top-3.5"></div>

                                    {/* Barra del proceso */}
                                    <div
                                        className={`absolute h-8 ${processColor} rounded-sm flex items-center px-2 shadow-sm border ${animationClass}`}
                                        style={{
                                            left: `${startPosition}%`,
                                            width: `${Math.max(width, 2)}%`,
                                            minWidth: "60px",
                                        }}
                                    >
                                        <span className="text-sm font-medium truncate text-white">
                                            {process.name}
                                            {" ["+process.id+"]"}
                                        </span>
                                    </div>

                                    {/* Tiempo de inicio */}
                                    <span
                                        className="absolute text-sm text-gray-500"
                                        style={{
                                            left: `${startPosition-1}%`,
                                            top: "90%",
                                        }}
                                    >
                                        {process.startTime}
                                    </span>

                                    {/* Tiempo de fin (si existe) */}
                                    {process.endTime && (
                                        <span
                                            className="absolute text-sm text-gray-500"
                                            style={{
                                                left: `${endPosition-3.5}%`,
                                                top: "90%",
                                            }}
                                        >
                                            {process.endTime}
                                        </span>
                                    )}
                                </div>
                            );
                        })}
                    </div>
                </div>
            </div>
        </div>
    );
}