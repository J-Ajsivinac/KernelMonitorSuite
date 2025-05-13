import {
    ResponsiveContainer,
    BarChart,
    XAxis,
    YAxis,
    Bar,
    CartesianGrid,
    Tooltip,
    Legend,
    Brush
} from "recharts";
import { useState, useEffect } from "react";

export const OptimizedProcessEnergyBarChart = ({ data, colors, id }) => {
    // Estados para filtrado y visualización
    const [displayData, setDisplayData] = useState([]);
    const [topCount, setTopCount] = useState(10); 
    
    // Colores para las barras
    const colorPrimary = colors?.[0] || "#8884d8";
    const colorFillStart = colors?.[1] || "#8884d8";
    const colorFillEnd = colors?.[2] || "#8884d8";

    // Procesar y ordenar los datos
    useEffect(() => {
        if (!data || data.length === 0) return;
        
        const processedData = data.map(process => ({
            ...process,
            name: `${process.pid}`,
            value: process.value / 1000000, // Convertir a un valor más legible
        }));

        // Ordenar los procesos por consumo de energía (de mayor a menor)
        const sortedData = [...processedData].sort((a, b) => b.value - a.value);
        
        // Mostrar solo los top N procesos
        setDisplayData(sortedData.slice(0, topCount));
    }, [data, topCount]);

    // Manejar cambio en el número de procesos a mostrar
    const handleTopCountChange = (e) => {
        setTopCount(Number(e.target.value));
    };

    return (
        <div className="flex flex-col w-full h-full p-2">
            <div className="flex items-center justify-end mb-4">
                <div className="flex items-center">
                    <label className="mr-2 text-sm">Mostrar top:</label>
                    <select 
                        value={topCount} 
                        onChange={handleTopCountChange}
                        className="px-2 py-1  text-white rounded border border-border-second-dark"
                    >
                        <option value={10}>10</option>
                        <option value={20}>20</option>
                        <option value={50}>50</option>
                        <option value={100}>100</option>
                        <option value={300}>Todos</option>
                    </select>
                </div>
            </div>
            
            <div className="flex-grow">
                <ResponsiveContainer width="100%" height="99%">
                    <BarChart 
                        data={displayData}
                        layout="vertical"
                        margin={{ top: 5, right: 30, left: 40, bottom: 5 }}
                    >
                        <defs>
                            <linearGradient id={`color-bar-${id}`} x1="0" y1="0" x2="1" y2="0">
                                <stop offset="0%" stopColor={colorFillStart} stopOpacity={0.9} />
                                <stop offset="100%" stopColor={colorFillEnd} stopOpacity={0.5} />
                            </linearGradient>
                        </defs>
                        <CartesianGrid strokeDasharray="3 3" opacity={0.1} horizontal={true} vertical={false} />
                        <XAxis 
                            type="number"
                            axisLine={false}
                            tickLine={false}
                            tickFormatter={(value) => `${value.toFixed(1)}`}
                        />
                        <YAxis
                            type="category"
                            dataKey="name"
                            axisLine={false}
                            tickLine={false}
                            width={40}
                            tick={{ fontSize: 12 }}
                        />
                        <Tooltip content={<CustomTooltip />} cursor={{ fill: "#151b1e" }}/>
                        <Legend 
                            verticalAlign="top" 
                            height={36}
                            formatter={() => 'Consumo de Energía (mJ)'}
                        />
                        <Bar 
                            dataKey="value" 
                            name="Consumo de Energía"
                            fill={`url(#color-bar-${id})`} 
                            stroke={colorPrimary}
                            strokeWidth={1}
                            radius={[0, 4, 4, 0]}
                        />
                    </BarChart>
                </ResponsiveContainer>
            </div>
        </div>
    );
};

function CustomTooltip({ active, payload }) {
    if (active && payload && payload.length) {
        const process = payload[0].payload;
        return (
            <div className="bg-panel-hover-dark border border-border-second-dark p-3 rounded shadow-lg">
                <h4 className="font-bold text-white mb-1">PID: {process.pid}</h4>
                <p className="text-gray-300 mb-1">Energía: {payload[0].value.toFixed(2)} mJ</p>
                <p className="text-gray-300 mb-1">CPU: {(process.cpu_usage / 1000000).toFixed(2)} ms</p>
                <p className="text-gray-300 mb-1">RAM: {(process.ram_usage / 1024).toFixed(2)} MB</p>
                <p className="text-gray-300 mb-1">I/O Lectura: {(process.io_read / 1024).toFixed(2)} KB</p>
                <p className="text-gray-300">I/O Escritura: {(process.io_write / 1024).toFixed(2)} KB</p>
            </div>
        );
    }
    return null;
}

export default OptimizedProcessEnergyBarChart;