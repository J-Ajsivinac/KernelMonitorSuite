import Content from "../components/Content";
import { Title } from "../components/Title";
import { TbNetwork } from "react-icons/tb";
import { AreaChartComp } from "../components/charts/AreaChart";
import { useEffect, useState } from "react";
import { requestNetwork } from "../api/requests";
import AreaDouble from "../components/charts/AreaDouble";

function Network() {
    const [data, setData] = useState([]);
    const totalDataPoints = 30;
    const intervalSeconds = 5;

    const getNetwork = async() =>{
        try {
            const res = await requestNetwork()
            let response = res.data
            console.log(response)
            const date = new Date();
            const timeString = date.toLocaleTimeString('es-GT', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                hour12: false
            });
            const value1 = response.bytes_sent_mb;
            const value2 = response.bytes_received_mb;

            setData(prev => {
                const updated = [...prev, { time: timeString, value1: value1,value2:value2 }];
                return updated.length > totalDataPoints ? updated.slice(-totalDataPoints) : updated;
            })

            // console.log(data,value1,value2)
        } catch (error) {
            console.log(error)
        }
    }

    useEffect(()=>{
        getNetwork();
        const interval = setInterval(getNetwork, intervalSeconds * 1000);
        return () => clearInterval(interval);
    },[])
        


    const colors = ["#838dc2","#333d53","#333d53"]
    return (
        <Content>
            <Title icon={<TbNetwork size={26} />} title={"Red"} />
            <span className="pl-7 mb-3">Tráfico de Red</span>
            {/* Bytes enviados y recibidos a lo largo del tiempo */}
            <div className="w-full h-full pl-6 pr-6 pb-4">
                <div className="w-full h-full border-1 border-border-dark rounded-sm">
                    <AreaDouble id='network' data={data} colors={[
                                    ["#838dc2", "#333d53", "#333d53"], // CPU
                                    ["#4e8f7b", "#2d4a48", "#2d4a48"], // RAM
                                ]}
                                names={['Bytes entrantes', 'Bytes salientes']}/>
                </div>
            </div>
        </Content>
    );
}

export default Network;
