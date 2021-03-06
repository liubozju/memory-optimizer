/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018 Intel Corporation
 *
 * Authors: Fengguang Wu <fengguang.wu@intel.com>
 *          Yao Yuan <yuan.yao@intel.com>
 */

#ifndef AEP_GLOBAL_SCAN_H
#define AEP_GLOBAL_SCAN_H

#include <vector>
#include <atomic>

#include "Queue.h"
#include "Process.h"
#include "EPTMigrate.h"
#include "BandwidthLimit.h"
#include "Sysfs.h"
#include "Numa.h"
#include "IntervalFitting.h"

enum JobIntent
{
  JOB_WALK,
  JOB_MIGRATE,
  JOB_QUIT,
};

typedef std::shared_ptr<EPTMigrate> EPTMigratePtr;

struct Job
{
  EPTMigratePtr migration;
  JobIntent intent;
};

struct threshold
{
  long value;
  long value_max;
};

class GlobalScan
{
  public:
    GlobalScan();

    void main_loop();
    void create_threads();
    void stop_threads();

    int collect();
    float walk_multi();
    float migrate();
    void progressive_profile();
    void count_refs();
    void count_migrate_stats();
#if 0
    void update_interval(bool finished);
#else
    void update_interval();
#endif
    void request_reload_conf();
    void apply_option();
    void prepare_walk_multi();

  private:
    void consumer_loop();
    int consumer_job(Job& job);
    void walk_once(int scans);
    bool should_stop_walk();
    void update_dram_free_anon_bytes();
    void reload_conf();
    bool exit_on_stabilized();
    bool exit_on_exceeded();
    bool check_exit_on_exceeded(pid_t pid);
    void update_pid_context();
    void get_memory_type();
    void calc_progressive_profile_parameter(ref_location from_type, int page_refs);
    void calc_migrate_count(long& promote_limit, long& demote_limit);
    void calc_migrate_parameter();

    unsigned long accept_hot_bytes()   { return dram_hot_target * 12 / 8; }
    unsigned long target_young_bytes() { return dram_hot_target * 10 / 8; }
    unsigned long target_hot_bytes()   { return dram_hot_target; }

    unsigned long get_dram_free_and_anon_bytes()
    { return get_dram_anon_bytes(true) << PAGE_SHIFT; }

    unsigned long get_dram_anon_bytes(bool is_include_free);

    unsigned long calc_migrated_bytes();
    void show_migrate_speed(float delta_time);
    bool is_all_migration_done();
    bool exit_on_converged();
    void anti_thrashing(EPTMigratePtr range, ProcIdlePageType type,
                        int anti_threshold);
    void init_migration_parameter(EPTMigratePtr range, ProcIdlePageType type);
    void calc_memory_size();
    void calc_hotness_drifting();
    void calc_page_hotness_drifting(EPTMigratePtr last, EPTMigratePtr current);
    void calc_global_threshold();
    bool in_adjust_ratio_stage();
    bool in_unbalanced_stage();
    bool should_target_aep_young();
    void save_scan_finish_ts();
    void save_context_last() {
      idle_ranges_last = idle_ranges;

      for (int i = 0; i < MAX_ACCESSED + 1; ++i)
        global_hot_threshold_last[i] = global_hot_threshold[i];
    }

  private:
    static const float MIN_INTERVAL;
    static const float MAX_INTERVAL;
    unsigned int nround;
    int nr_walks;
    int nr_acceptable_scans;
    float interval;
    float real_interval;
    struct timeval last_scan_start;
    unsigned long young_bytes;
    unsigned long pmem_young_bytes;
    unsigned long top_bytes;
    unsigned long all_bytes;
    unsigned long dram_free_anon_bytes;
    unsigned long dram_hot_target;
    unsigned long nr_total_scans = 0;

    ProcessCollection process_collection;
    std::vector<std::shared_ptr<EPTMigrate>> idle_ranges;
    std::vector<std::shared_ptr<EPTMigrate>> idle_ranges_last;
    std::vector<std::thread> worker_threads;
    Queue<Job> work_queue;
    Queue<Job> done_queue;

    std::atomic_int conf_reload_flag;

    BandwidthLimit throttler;
    NumaNodeCollection numa_collection;
    ProcVmstat proc_vmstat;
    Sysfs sysfs;

    IntervalFitting<float, unsigned long, 5> intervaler[2];

    struct threshold global_hot_threshold[MAX_ACCESSED + 1];
    struct threshold global_hot_threshold_last[MAX_ACCESSED + 1];

    long total_pmem_kb[MAX_ACCESSED + 1];
    long total_dram_kb[MAX_ACCESSED + 1];
    long total_mem_kb[MAX_ACCESSED + 1];

    long global_total_pmem_kb = 0;
    long global_total_dram_kb = 0;
    long global_total_mem_kb = 0;
    long global_dram_ratio = 0;
};

#endif
// vim:set ts=2 sw=2 et:
