import React from "react";

const ProgressBar = ({
    value,
    color = "blue",
    className = "",
    gap = 4,
    showPercentage = true,
    percentageClassName = "",
}) => {
    // Color mapping
    const colorMap = {
        blue: "bg-[#818bc0]/60 border-[#818bc0]",
        green: "bg-[#4a8e75]/60 border-[#4a8e75]",
        red: "bg-red-500",
        purple: "bg-purple-500",
        yellow: "bg-yellow-500",
        pink: "bg-pink-500",
        indigo: "bg-indigo-500",
    };

    // Get the appropriate color class
    const colorClass = colorMap[color] || colorMap.blue;

    // Ensure value is between 0 and 100
    const progressValue = Math.min(100, Math.max(0, value));

    // Calculate widths accounting for gap
    const totalGapWidth = gap; // px
    const gapPercentage = totalGapWidth / 2;

    // Adjust width calculations to account for the gap
    const completedWidth =
        progressValue > 0
            ? `calc(${progressValue}% - ${gapPercentage}px)`
            : "0%";
    const remainingWidth =
        progressValue < 100
            ? `calc(${100 - progressValue}% - ${gapPercentage}px)`
            : "0%";

    // Format percentage for display
    const formattedPercentage = `${progressValue.toFixed(2)}%`;

    return (
        <div className={`w-full ${className} -z-10`}>
            <div className="relative h-4 w-full overflow-hidden rounded-sm">
                {/* Completed section */}
                <div
                    className={`absolute left-0 top-0 h-full rounded-l-sm border-2 ${colorClass} transition-all duration-300 ease-out`}
                    style={{ width: completedWidth }}
                ></div>

                {/* Remaining section */}
                <div
                    className="absolute right-0 top-0 h-full rounded-r-sm border-2 border-border-second-dark bg-panel-dark transition-all duration-300 ease-out"
                    style={{ width: remainingWidth }}
                ></div>
            </div>

            {/* Percentage text below progress bar */}
            {showPercentage && (
                <div
                    className={`mt-1 text-sm text-gray-300 ${percentageClassName}`}
                >
                    {formattedPercentage}
                </div>
            )}
        </div>
    );
};

export default ProgressBar;
