import axios from "./axios";

export const requestResources = () => axios.get('/resources')
export const requestProcess = () => axios.get('/process')
export const requestTimeLine = () => axios.get('/process/time')
export const requestNetwork = () => axios.get('/network')
export const requestEnergy = () => axios.get('/energy')
export const requestSearchPID = (pid) => axios.post('/process/search',pid)
export const requestKillPID = (pid) => axios.post('/processes/kill',pid)