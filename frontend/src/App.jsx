import { TbDeviceHeartMonitorFilled } from "react-icons/tb";
import Sidebar, { SidebarItem } from "./components/Sidebar";
import { BrowserRouter, Routes, Route } from 'react-router-dom'
import { RiCpuLine } from "react-icons/ri";
import { TbNetwork } from "react-icons/tb";
import { SlEnergy } from "react-icons/sl";
import { FaLaptopCode } from "react-icons/fa";
import Index from "./pages/Index";
import Network from "./pages/Network";
import Energy from "./pages/Energy";
import Resources from "./pages/Resources";
import { Toaster } from "sonner";

function App() {
    return(
        <BrowserRouter>
            <Toaster position="top-center" richColors theme="dark" />
            <div className="flex">
                <Sidebar>
                    <SidebarItem to="/" icon={<RiCpuLine size={20} />} text="Procesos" />
                    <SidebarItem to="/resources" icon={<FaLaptopCode size={20} />} text="Recursos" />
                    <SidebarItem to="/network" icon={<TbNetwork size={20} />} text="Red" />
                    <SidebarItem to="/energy" icon={<SlEnergy size={20} />} text="Energía" />
                </Sidebar>
                <div className="flex flex-col items-center justify-center h-screen w-full">
                    <Routes>
                        <Route path='/' element={<Index/>} />
                        <Route path='/resources' element={<Resources/>} />
                        <Route path='/network' element={<Network/>} />
                        <Route path='/energy' element={<Energy/>} />
                    </Routes>
                </div>
            </div>
        </BrowserRouter>
    )

}

export default App; 
