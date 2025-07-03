export interface RamProcess {
  pid: number;
  name: string;
  ram_kb: number;
}

export interface RamStats {
  total_gb: number;
  used_gb: number;
  percent_used: number;
}