import Content from "../components/Content";
import { Title } from "../components/Title";
import { SlEnergy } from "react-icons/sl";
import { AreaChartComp } from "../components/charts/AreaChart";
import { useEffect, useState } from "react";
import { requestEnergy } from "../api/requests";
import BarChartCustom, { OptimizedProcessEnergyBarChart } from "../components/charts/BarChart";

function Energy() {
    const [data, setData] = useState([]);

    const getEnergy = async () => {
        try {
            const res = await requestEnergy()
            // const date = new Date();
            // const timeString = date.toLocaleTimeString('es-GT', {
            //     hour: '2-digit',
            //     minute: '2-digit',
            //     second: '2-digit',
            //     hour12: false
            // });
            let response = res.data
            // console.log(response)
            setData(response)
        } catch (error) {
            console.log(error)
        }
    }

    useEffect(()=>{
        getEnergy();
    },[])
    
    const colors = ["#4e8f7b","#2d4a48","#2d4a48"]
    return (
        <Content>
            <Title icon={<SlEnergy size={26} />} title={"Energía"} />
            <span className="pl-7 mb-3">Consumo de energía por proceso</span>
            <div className="w-full h-full pl-6 pr-6 pb-4 ">
                <div className="flex w-full h-[95%] border-1 border-border-dark rounded-sm  ">
                    <OptimizedProcessEnergyBarChart data={data} colors={colors} id={"energy-chart"} />
                </div>
            </div>
        </Content>
    );
}

export default Energy;
