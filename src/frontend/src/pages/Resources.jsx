import Content from "../components/Content";
import { Title } from "../components/Title";
import { TbNetwork } from "react-icons/tb";
import { TitleContainer } from "../components/TitleContainer";
import { FaLaptopCode } from "react-icons/fa";
import { AreaChartComp } from "../components/charts/AreaChart";
import { FaMemory } from "react-icons/fa";
import { RiCpuLine } from "react-icons/ri";
import { requestResources } from "../api/requests";
import { useEffect, useState } from "react";

function Resources() {
    const [cpuData, setCpuData] = useState([]);
    const [ramData, setRamData] = useState([]);
    const [ramPer, setRamPer] = useState(0);
    const [cpuPer, setCpuPer] = useState(0);
    const totalDataPoints = 30;
    const intervalSeconds = 5;

    const getResources = async () => {
        try {
            const res = await requestResources(); 
            const date = new Date();
            const timeString = date.toLocaleTimeString('es-GT', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                hour12: false
            });
            
            
            let response = res.data
            // console.log(response)
            const cpuPercentage = response?.CPU?.percentage ?? 0;
            const ramPercentage = response?.RAM?.percentage ?? 0;

            setRamPer(ramPercentage);
            setCpuPer(cpuPercentage);

            setCpuData(prev => {
                const updated = [...prev, { time: timeString, value: cpuPercentage }];
                return updated.length > totalDataPoints ? updated.slice(-totalDataPoints) : updated;
            });

            setRamData(prev => {
                const updated = [...prev, { time: timeString, value: ramPercentage }];
                return updated.length > totalDataPoints ? updated.slice(-totalDataPoints) : updated;
            });
        } catch (error) {
            console.log("Error obteniendo recursos:", error);
        }
    };

    useEffect(() => {
        getResources();
        const interval = setInterval(getResources, intervalSeconds * 1000);
        return () => clearInterval(interval);
    }, []);

    const colorsCPU = ["#838dc2","#333d53","#333d53"]
    const colorsRAM = ["#4e8f7b","#2d4a48","#2d4a48"]
    return (
        <Content>
            <Title icon={<FaLaptopCode size={26} />} title={"Recursos"} />
            {/* Bytes enviados y recibidos a lo largo del tiempo */}
            <div className="w-full h-full pl-6 pr-6 pb-4">
                <div className="flex flex-col h-[97%] w-full gap-4">
                    <div className="flex h-1/2 flex-col w-full border-1 border-border-second-dark rounded-sm pt-3 pl-3 pb-2">
                        <div className="flex items-center justify-between pr-3 mb-4">
                            <TitleContainer
                                icon={<RiCpuLine size={24} />}
                                title={"Uso de CPU"}
                            />
                            <span className="text-3xl font-semibold">{cpuPer + "%"}</span>
                        </div>
                        <AreaChartComp data={cpuData} colors={colorsCPU} id={1} />
                    </div>
                    <div className="flex h-1/2 flex-col w-full border-1 border-border-second-dark rounded-sm p-3">
                        <div className="flex items-center justify-between pr-3 mb-4">
                            <TitleContainer
                                icon={<FaMemory size={24} />}
                                title={"Uso de RAM"}
                            />
                            <span className="text-3xl font-semibold">{ramPer + "%"}</span>
                        </div>
                        <AreaChartComp data={ramData} colors={colorsRAM} id={2} />
                    </div>
                </div>
            </div>
        </Content>
    );
}

export default Resources;
