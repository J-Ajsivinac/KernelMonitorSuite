const typeColors = {
    Running: "bg-background-success/30 text-text-accent", 
    Sleeping: "bg-[#5d68c8]/30 text-[#5d68c8]",     
    "Disk Sleep": "bg-[#5d68c8]/30 text-[#5d68c8]",   
    Zombie: "bg-[#de8978]/30 text-[#de8978]",   
    Stopped: "bg-[#c02841]/30 text-[#c02841]",   
    Tracing: "bg-[#a9acff]/30 text-[#a9acff]",  
    Dead: "bg-background-error/30 text-background-error", 
    Wakekill: "bg-background-error/30 text-background-error",  
    default: "bg-[#afafaf]/30 text-[#afafaf]",   
};

function Tag({ content, type="Running" }) {
    const colorClass = typeColors[type] || typeColors.default;

    return (
        <span
            className={`px-3.5 py-1 rounded-md w-fit text-sm  ${colorClass}`}
        >
            {content}
        </span>
    );
}

export default Tag;