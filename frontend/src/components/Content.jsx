function Content({ children }) {
    return (
        <div className="w-full flex flex-grow justify-center pt-5">
            <div className="flex flex-col w-full">{children}</div>
        </div>
    );
}

export default Content;