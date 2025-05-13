import React, { useState } from "react";

const TabComponent = ({ children }) => {
    const [openTab, setOpenTab] = useState(0); // Inicia en la primera pestaña (índice 0)

    return (
        <div className="font-sans flex items-start justify-center w-full h-full">
                <div className="w-full h-full">
                    {/* Botones de las pestañas */}
                    <div className="mb-4 pl-6 flex border-b-1 border-border-dark">
                        {children.map((child, index) => (
                            <button
                                key={index}
                                onClick={() => setOpenTab(index)}
                                className={`py-2 px-4 text-lg focus:outline-none transition-all duration-150 cursor-pointer ${
                                    openTab === index
                                        ? "text-text-accent border-b-1"
                                        : "text-text-second-dark"
                                }`}
                            >
                                <span className="flex items-center gap-2 justify-center">
                                {child.props.icon}
                                {child.props.label}{" "}
                                </span>
                                {/* Muestra el label de la pestaña */}
                            </button>
                        ))}
                    </div>

                    {/* Contenido de las pestañas */}
                    <div className="transition-all duration-300 rounded-lg h-[calc(100vh-9rem)]">
                        {children[openTab]}{" "}
                        {/* Renderiza el contenido de la pestaña activa */}
                    </div>
                </div>
        </div>
    );
};

export default TabComponent;
