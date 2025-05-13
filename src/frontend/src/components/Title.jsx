export function Title({icon, title}) {
    return (
        <div className="flex items-center gap-2 mb-4 ml-6">
            <span className="text-text-accent/70">{icon}</span>
            <span className="text-3xl font-medium text-text-accent">{title}</span>
        </div>
    )
}