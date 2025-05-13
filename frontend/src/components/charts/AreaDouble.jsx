import {
    ResponsiveContainer,
    AreaChart,
    XAxis,
    YAxis,
    Area,
    CartesianGrid,
    Tooltip,
    ReferenceLine,
} from "recharts";

export const AreaDouble = ({ data, colors, id, names }) => {
    // Desestructurar los colores para cada dataset
    const [colorPrimary1, colorFillStart1, colorFillEnd1] = colors[0]; // Colores para el primer dataset (ej. CPU)
    const [colorPrimary2, colorFillStart2, colorFillEnd2] = colors[1]; // Colores para el segundo dataset (ej. RAM)

    const [name1, name2] = names || ["Value 1", "Value 2"];

    return (
        <ResponsiveContainer width="100%" height="100%">
            <AreaChart data={data}>
                {/* Definición de gradientes para ambos datasets */}
                <defs>
                    <linearGradient id={`color-${id}-1`} x1="0" y1="0" x2="0" y2="1">
                        <stop offset="0%" stopColor={colorFillStart1} stopOpacity={0.9} />
                        <stop offset="80%" stopColor={colorFillEnd1} stopOpacity={0.5} />
                    </linearGradient>
                    <linearGradient id={`color-${id}-2`} x1="0" y1="0" x2="0" y2="1">
                        <stop offset="0%" stopColor={colorFillStart2} stopOpacity={0.9} />
                        <stop offset="80%" stopColor={colorFillEnd2} stopOpacity={0.5} />
                    </linearGradient>
                </defs>

                {/* Primer área (ej. CPU) */}
                <Area 
                    dataKey="value1" 
                    stroke={colorPrimary1} 
                    fill={`url(#color-${id}-1)`} 
                    name={name1}// Nombre para el tooltip
                />
                
                {/* Segunda área (ej. RAM) */}
                <Area 
                    dataKey="value2" 
                    stroke={colorPrimary2} 
                    fill={`url(#color-${id}-2)`} 
                    name={name2} // Nombre para el tooltip
                />

                <ReferenceLine x="Page C" stroke="red" label="Max PV PAGE" />
                <XAxis
                    dataKey="time"
                    axisLine={false}
                    tickLine={false}
                    hide 
                />

                <YAxis
                    orientation="right"
                    dataKey="value1" // Usamos uno de los dataKeys como referencia
                    axisLine={false}
                    tickLine={false}
                    tickFormatter={(number) => `${number.toFixed(2)}`}
                />

                <Tooltip content={<CustomTooltip />} />
                
                <CartesianGrid opacity={0.1} vertical={false} stroke={'#ffffff'} strokeDasharray="10 10" />
            </AreaChart>
        </ResponsiveContainer>
    );
};

function CustomTooltip({ active, payload, label }) {
    if (active && payload && payload.length) {
        // Determinar si estamos mostrando CPU/RAM (buscando esos nombres en los payloads)
        const isCpuRam = payload.some(
            entry => entry.name.includes('CPU') || entry.name.includes('RAM')
        );

        return (
            <div className="bg-panel-hover-dark border-2 border-border-second-dark p-4 text-center rounded-sm">
                <h4 className='font-bold'>
                    {label.includes('T') ? label.split('T')[1].substr(0, 8) : label}
                </h4>
                {payload.map((entry, index) => (
                    <p key={`item-${index}`} style={{ color: entry.color }}>
                        {entry.name}: {entry.value.toFixed(2)}{isCpuRam ? '%' : 'MB'}
                    </p>
                ))}
            </div>
        );
    }
    return null;
}

export default AreaDouble;