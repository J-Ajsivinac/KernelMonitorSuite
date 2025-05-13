import Content from "../components/Content";
import InputField from "../components/InputField";
import TabComponent from "../components/TabComponent";
import TabContent from "../components/TabContent";
import { Title } from "../components/Title";
import { MdDeleteOutline } from "react-icons/md";
import { TitleContainer } from "../components/TitleContainer";
import { RiCpuLine } from "react-icons/ri";
import { MdTimeline } from "react-icons/md";
import ProcessTimeline from "../components/charts/TimeLineChart";
    import { LuAppWindow } from "react-icons/lu";
import { useEffect, useRef, useState } from "react";
import AreaDouble from "../components/charts/AreaDouble";
import ProgressBar from "../components/ProgressBar";
import { toast } from "sonner";
import { FiCheck } from "react-icons/fi";
import Tag from "../components/Tag";
import { requestKillPID, requestProcess, requestSearchPID, requestTimeLine } from "../api/requests";

function Index() {
    const [selectedProcess, setSelectedProcess] = useState("");
    const [selectedPids, setSelectedPids] = useState([]);
    const [searchTerm, setSearchTerm] = useState("");
    // Estado para el valor de búsqueda
    const [searchValue, setSearchValue] = useState("");
    const [processData, setProcessData] = useState([]);
    const intervalRef = useRef(null);
    const [timeLine, setTimeLine] = useState([]);

    const intervalSeconds = 5;
    const totalDataPoints = 30;

    const [graphData, setGraphData] = useState([]);

    const getAllProcess = async ()=>{
        try {
            const res = await requestProcess();
            let response = res.data
            setProcessData(response)
        } catch (error) {
            console.log(error)
        }
    }

    const getTimeLine = async ()=>{
        try {
            const res = await requestTimeLine();
            let response = res.data
            console.log(response)
            // setProcessData(response)
            setTimeLine(response)
        } catch (error) {
            console.log(error)
        }
    }

    useEffect(()=>{
        getAllProcess();
        const interval = setInterval(getAllProcess, intervalSeconds * 1000);
        return () => clearInterval(interval);
    },[])

    useEffect(()=>{
        getTimeLine();
        const interval = setInterval(getTimeLine, intervalSeconds * 1000);
        return () => clearInterval(interval);
    },[])


    // Datos de ejemplo para la tabla
    const process = [
        { icon: <LuAppWindow />, nombre: "Chrome", pid: 12 },
        { icon: <LuAppWindow />, nombre: "Brave", pid: 33 },
        { icon: <LuAppWindow />, nombre: "Docker", pid: 334 },
        { icon: <LuAppWindow />, nombre: "Docker", pid: 334 },
        { icon: <LuAppWindow />, nombre: "Docker", pid: 334 },
    ];

    // Filtrar procesos basados en el término de búsqueda
    const filteredProcessData = processData.filter((proc) => {
        if (!searchTerm) return true;
        const term = searchTerm.toLowerCase();
        return (
            proc.pid.toString().includes(searchTerm) ||
            (proc.nombre && proc.nombre.toLowerCase().includes(term)) ||
            (proc.user && proc.user.toLowerCase().includes(term))
        );
    });

    const handleSelectProcess = (value) => {
        if (value) {
            setSelectedProcess(value);
        }
    };

    const handleSearch = () => {
        // Ejecutar búsqueda inmediata
        searchPID();

        // Limpiar interval anterior si existía
        if (intervalRef.current) {
            clearInterval(intervalRef.current);
        }

        // Iniciar nuevo intervalo
        intervalRef.current = setInterval(() => {
            searchPID();
        }, intervalSeconds * 1000);
    };

    const searchPID = async()=>{
        // if(searchValue == ""){
        //     return;
        // }
        const objectSearched = {
            pid: parseInt(searchValue, 10)
        }
        console.log(objectSearched)

        try {
            const res = await requestSearchPID(objectSearched);
            let response = res.data
            const date = new Date();
            const timeString = date.toLocaleTimeString('es-GT', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                hour12: false
            });
            
            const cpuPercentage = response?.cpu ?? 0;
            const ramPercentage = response?.ram ?? 0;

            setGraphData(prev => {
                const updated = [...prev, { time: timeString, value1: cpuPercentage, value2: ramPercentage}];
                return updated.length > totalDataPoints ? updated.slice(-totalDataPoints) : updated;
            });

        } catch (error) {
            console.log(error)
        }
    }

    useEffect(() => {
        // Limpiar intervalo al desmontar el componente
        return () => {
            if (intervalRef.current) {
                clearInterval(intervalRef.current);
            }
        };
    }, []);

    const handleCheckboxChange = (pid) => {
        setSelectedPids((prev) => {
            if (prev.includes(pid)) {
                return prev.filter((id) => id !== pid);
            } else {
                return [...prev, pid];
            }
        });
    };

    const handleKillProcesses = () => {
        if (selectedPids.length === 0) return;
        console.log(selectedPids)
        selectedPids.forEach(async (pid) => { 
            let format = {
                pid: pid,
                signal: 9
            }
            console.log(format)
            try {
                const res = await requestKillPID(format);
                console.log(res)
            } catch (error) {
                toast.error("Ocurrio un error al usar SIGKILL ", {
                    duration: 2000,
                });
            }
        });
        toast.success("Procesos seleccionados han sido eliminados", {
            duration: 2000,
        });
        // Aquí iría la lógica para matar los procesos
        setSelectedPids([]); // Limpiar selección después de matar procesos
    };

    return (
        <Content>
            <Title icon={<RiCpuLine size={26} />} title={"Procesos"} />
            <TabComponent>
                {/*============================
                    TAB: Procesos
                =============================*/}
                <TabContent label="Resumen">
                    <div className="flex px-6 py-1 flex-col gap-4 h-full">
                        <form action="" className="flex justify-between w-full">
                            <div className="w-1/3">
                                <InputField
                                    name="search"
                                    value={searchTerm}
                                    onChange={(e) =>
                                        setSearchTerm(e.target.value)
                                    }
                                    placeholder="Buscar por PID o nombre..."
                                />
                            </div>
                            <button
                                type="button"
                                onClick={handleKillProcesses}
                                disabled={selectedPids.length === 0}
                                className={`flex items-center gap-1.5 px-3 rounded-sm cursor-pointer transition-all duration-200 ${
                                    selectedPids.length > 0
                                        ? "bg-background-error/60 hover:bg-background-error/80"
                                        : "bg-background-error/30 cursor-not-allowed"
                                }`}
                            >
                                <MdDeleteOutline size={18} /> Matar Procesos
                            </button>
                        </form>
                        <div className="flex flex-grow h-full w-full gap-2">
                            <div className="w-full border-1 border-border-second-dark rounded-sm overflow-hidden">
                                <div className="max-h-[calc(100vh-220px)] overflow-y-auto">
                                    <table className="table w-full text-gray-400 text-sm border-collapse">
                                        <thead className="bg-[#151b1e] text-[#b2b8bb] sticky top-0 z-10">
                                            <tr className="">
                                                <th className="pl-3 py-3 pr-0 text-left font-semibold"></th>
                                                <th className="p-3 text-left font-semibold">
                                                    PID
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    NOMBRE DEL PROCESO
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    USUARIO
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    ESTADO
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    CPU %
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    RAM %
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    Hilos
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    PRIORIDAD
                                                </th>
                                                <th className="p-3 text-left font-semibold">
                                                    INICIO
                                                </th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            {filteredProcessData.map((proc) => (
                                                <tr
                                                    key={proc.pid}
                                                    className="bg-transparent border-b-1 border-border-dark text-text-dark hover:bg-gray-900/20"
                                                >
                                                    <td className="px-3 py-2 text-left">
                                                        <div className="flex">
                                                            <input
                                                                type="checkbox"
                                                                id={`checkbox-${proc.pid}`}
                                                                checked={selectedPids.includes(
                                                                    proc.pid
                                                                )}
                                                                onChange={() =>
                                                                    handleCheckboxChange(
                                                                        proc.pid
                                                                    )
                                                                }
                                                                className="peer hidden"
                                                            />
                                                            <label
                                                                htmlFor={`checkbox-${proc.pid}`}
                                                                className="select-none cursor-pointer rounded border-2 border-gray-400/60
                                                                w-5 h-5 flex items-center justify-center transition-colors duration-200 ease-in-out 
                                                                peer-checked:bg-blue-500/40 peer-checked:border-blue-500"
                                                            >
                                                                <FiCheck
                                                                    className={`text-white transition-opacity duration-200 ease-in-out ${
                                                                        selectedPids.includes(
                                                                            proc.pid
                                                                        )
                                                                            ? "opacity-100"
                                                                            : "opacity-0"
                                                                    }`}
                                                                    size={14}
                                                                />
                                                            </label>
                                                        </div>
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        {proc.pid}
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        <div className="flex items-center gap-2">
                                                            {<LuAppWindow />}{" "}
                                                            {proc.name}
                                                        </div>
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        {proc.user}
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        <Tag
                                                            content={
                                                                proc.state
                                                            }
                                                            type={proc.state}
                                                        />
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        <ProgressBar
                                                            value={proc.cpu}
                                                            color="blue"
                                                        />
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        <ProgressBar
                                                            value={proc.ram}
                                                            color="green"
                                                        />
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        {proc.threads}
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        {proc.priority-100}
                                                    </td>
                                                    <td className="px-3 py-4 text-left">
                                                        {proc.start_time}
                                                    </td>
                                                </tr>
                                            ))}
                                        </tbody>
                                    </table>
                                </div>
                            </div>
                        </div>
                    </div>
                </TabContent>

                {/*============================
                    TAB: Detalle
                =============================*/}
                <TabContent label="Detalle">
                    <div className="flex flex-col h-full w-full px-6 py-1 gap-4">
                        <div className="flex flex-col w-full">
                            <InputField
                                name="search_one"
                                placeholder="Buscar por PID o nombre..."
                                onChange={(e) => setSearchValue(e.target.value)}
                                value={searchValue}
                                onKeyDown={(e) => {
                                    if (e.key === "Enter") {
                                        setGraphData([]);
                                        handleSearch();
                                    }
                                }}
                            />
                        </div>
                        <div className="flex h-full gap-4 border-1 border-border-second-dark rounded-sm">
                            <AreaDouble
                                id="cpu-ram-usage"
                                data={graphData}
                                colors={[
                                    ["#838dc2", "#333d53", "#333d53"], // CPU
                                    ["#4e8f7b", "#2d4a48", "#2d4a48"], // RAM
                                ]}
                                names={['CPU', 'RAM']}
                            />
                        </div>
                    </div>
                </TabContent>

                {/*============================
                    TAB: Línea del tiempo
                =============================*/}
                <TabContent label="Línea del Tiempo">
                    <div className="flex px-6 flex-col py-1 gap-4 h-full">
                        <div className="h-[98%] w-full border-1 border-border-second-dark rounded-sm p-3">
                            <TitleContainer
                                icon={<MdTimeline size={24} />}
                                title={"Línea de Tiempo de Procesos"}
                            />
                            <ProcessTimeline processes={timeLine} />
                        </div>
                    </div>
                </TabContent>
            </TabComponent>
        </Content>
    );
}

export default Index;
