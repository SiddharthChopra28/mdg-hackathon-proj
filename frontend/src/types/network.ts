export interface NetworkData {
  total: number;
  incoming: number;
  outgoing: number;
  external: number;
  local: number;
}

export interface AppUsage {
  app_name: string;
  download_bytes: number; // in KB
  upload_bytes: number;
}

export interface TrafficType {
  type: string;
  usage: number; // in KB
}