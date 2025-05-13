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
// import { format, parseISO } from "date-fns";

export const AreaChartComp = ({data,colors,id}) => {
    let colorPrimary =  colors[0];
    let colorFillStart =colors[1];
    let colorFillEnd = colors[2];

    return (
        <ResponsiveContainer width="100%" height="100%">
            <AreaChart data={data}>
                <defs>
                    <linearGradient id={`color-${id}`} x1="0" y1="0" x2="0" y2="1">
                        <stop offset="0%" stopColor={colorFillStart} stopOpacity={0.9} />
                        <stop offset="80%" stopColor={colorFillEnd} stopOpacity={0.5} />
                    </linearGradient>
                </defs>

                <Area dataKey="value" stroke={colorPrimary} fill={`url(#color-${id})`}/>
                <ReferenceLine x="Page C" stroke="red" label="Max PV PAGE" />
                <XAxis
                    dataKey="time"
                    axisLine={false}
                    tickLine={false}
                    hide 
                />

                <YAxis
                    orientation="right"
                    datakey="value"
                    axisLine={false}
                    tickLine={false}
                    // tickCount={8}
                    tickFormatter={(number) => `${number.toFixed(2)}`}
                />

                <Tooltip content={<CustomTooltip />} />
                
                <CartesianGrid opacity={0.1} vertical={false} stroke={'#ffffff'} strokeDasharray="10 10" />
            </AreaChart>
        </ResponsiveContainer>
    );
};

export default AreaChart;


function CustomTooltip({ active, payload, label }) {
    if (active && payload && payload.length) {
        return (
            <div className="bg-panel-hover-dark border-2 border-border-second-dark p-4 text-center rounded-sm">
                <h4 className='font-bold'>
                    {/* Muestra solo la hora (HH:MM:SS) */}
                    {label.includes('T') ? label.split('T')[1].substr(0, 8) : label}
                </h4>
                <p>
                    Uso: {payload[0].value.toFixed(2)}%
                </p>
            </div>
        );
    }
    return null;
}
