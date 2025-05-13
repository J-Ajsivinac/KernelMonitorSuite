export function TitleContainer({icon, title}) {
    return (
        <div className="flex items-center gap-2">
            <span className="text-text-accent">{icon}</span>
            <span className="text-lg text-text-dark">{title}</span>
        </div>
    )
}