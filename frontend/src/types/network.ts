export interface NetworkData {
  total: number;
  incoming: number;
  outgoing: number;
  external: number;
  local: number;
}

export interface AppUsage {
  name: string;
  usage: number; // in KB
}

export interface TrafficType {
  type: string;
  usage: number; // in KB
}