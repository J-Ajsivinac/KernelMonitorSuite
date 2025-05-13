import { LuChevronFirst, LuChevronLast } from "react-icons/lu";
import { createContext, useContext, useState } from "react";
import { NavLink } from "react-router-dom"; // Importa NavLink
import { BiChart } from "react-icons/bi";

const SidebarContext = createContext();

export default function Sidebar({ children }) {
    const [expanded, setExpanded] = useState(true);
    return (
        <>
            <aside className="h-screen">
                <nav className="h-full flex flex-col bg-background-dark border-border-dark border-r shadow-sm">
                    <div className="p-4 pb-2 flex justify-between items-center">
                        <div
                            className={`flex items-center overflow-hidden transition delay-75 duration-150 text-text-dark ${
                                expanded ? "w-fit" : "w-0"
                            }`}
                        >
                            <BiChart size={28} className="w-10" />
                            <span className="text-md">Process Monitor</span>
                        </div>
                        <button
                            onClick={() => setExpanded((curr) => !curr)}
                            className="p-1.5 rounded-lg hover:bg-panel-hover-dark text-text-light transition delay-75 duration-150 cursor-pointer"
                        >
                            {expanded ? (
                                <LuChevronFirst size={24} />
                            ) : (
                                <LuChevronLast size={24} />
                            )}
                        </button>
                    </div>

                    <SidebarContext.Provider value={{ expanded }}>
                        <ul className="flex-1 px-3">{children}</ul>
                    </SidebarContext.Provider>
                </nav>
            </aside>
        </>
    );
}

export function SidebarItem({ icon, text, to, alert }) {
    const { expanded } = useContext(SidebarContext);
    return (
        <li className="relative">
            <NavLink
                to={to}
                className={({ isActive }) =>
                    `flex items-center py-2 px-3 my-1 font-medium rounded-sm cursor-pointer transition-colors group ${
                        isActive
                            ? "bg-text-accent/20 text-text-accent"
                            : "text-white/60"
                    }`
                }
            >
                {icon}
                <span
                    className={`overflow-hidden transition-all ${
                        expanded ? "w-44 ml-3" : "w-0"
                    }`}
                >
                    {text}
                </span>
                {!expanded && (
                    <div
                        className={`z-30 absolute left-full rounded-md px-3 py-2 ml-6 bg-[#14362c] text-white invisible opacity-20 -translate-x-3 transition-all group-hover:visible group-hover:opacity-100 group-hover:translate-x-0`}
                    >
                        {text}
                    </div>
                )}
            </NavLink>
        </li>
    );
}
