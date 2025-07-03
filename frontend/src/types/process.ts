export interface ProcessData {
  name: string;
  cpu_time: number;
  pid_count: number;
  cpu_percent: number;
}

export interface ProcessorInfo {
  model_name: string;
  base_speed_ghz: number;
  core_count: number;
}

export type PriorityLevel = 'high' | 'med' | 'low';