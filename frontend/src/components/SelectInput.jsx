import { useState, useEffect } from "react";
import { TbSelector } from "react-icons/tb";
import { motion, AnimatePresence } from "framer-motion";
import { TiArrowUnsorted } from "react-icons/ti";

export function SelectInput({ options, placeHolder, onSelectOption, value, haveBackground = false, icon: Icon }) {
    const [openselect, setOpenSelect] = useState(false);
    const [selectedOption, setSelectedOption] = useState(value);

    useEffect(() => {
        setSelectedOption(value);
    }, [value]);

    function selectvalue(option) {
        setSelectedOption(option);
        setOpenSelect(false);
        onSelectOption(option);
    }

    function toggleSelect() {
        setOpenSelect(prev => !prev);
    }

    const dropdownVariants = {
        hidden: {
            opacity: 0,
            y: -10,
            transition: {
                duration: 0.2,
                ease: "easeInOut"
            }
        },
        visible: {
            opacity: 1,
            y: 0,
            transition: {
                duration: 0.2,
                ease: "easeInOut"
            }
        }
    };

    function openOption() {
        setOpenSelect(true);
    }


    return (
        <div className="flex w-full relative">
            <div className={`flex items-center pl-2 rounded-l-sm border-l border-y border-border-second-dark`}>  
                {Icon && <span className=""><Icon className="w-5 h-5 text-gray-400 left-3 top-3" /></span>}
            </div>
            <input
                value={selectedOption ? selectedOption.nombre : ''}
                onClick={toggleSelect}
                onBlur={() => {
                    setTimeout(() => setOpenSelect(false), 150);
                }}
                id="league"
                type="text"
                className={`h-11.5 px-2 py-4 w-full placeholder:text-gray-400 text-text-dark outline-none border-y border-border-second-dark rounded-none cursor-pointer`}
                placeholder={placeHolder}
                readOnly
            />
            <div
                tabIndex={0}
                onBlur={() => {
                    setOpenSelect(false);
                }}
                className={`flex pr-2 items-center justify-center rounded-e-md h-11.5 border-y border-r border-border-second-dark`}
            >
                <TbSelector size={22} className="text-text-light dark:text-text-dark" />
            </div>

            <AnimatePresence>
                {openselect && (
                    <motion.div
                        initial="hidden"
                        animate="visible"
                        exit="hidden"
                        variants={dropdownVariants}
                        className="absolute w-full py-2 translate-y-14 max-h-48 overflow-y-auto overflow-x-hidden bg-panel-dark z-10 px-2 rounded-sm border border-gray-600 shadow-lg"
                    >
                        {options.map((item, index) => (
                            <motion.li 
                                whileHover={{ scale: 1.014 }}
                                whileTap={{ scale: 0.98 }}
                                onMouseDown={() => selectvalue(item)} 
                                key={index} 
                                className="flex items-center gap-3 z-20 list-none text-text-second-dark font-semibold hover:text-text-light hover:dark:text-text-dark px-2 py-1 cursor-pointer"
                            >
                                {item.icon}
                                {item.nombre}
                            </motion.li>
                        ))}
                    </motion.div>
                )}
            </AnimatePresence>
        </div>
    );
}