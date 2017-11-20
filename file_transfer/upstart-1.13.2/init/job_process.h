/* upstart
 *
 * Copyright  2009,2010,2011 Canonical Ltd.
 * Author: Scott James Remnant <scott@netsplit.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INIT_JOB_PROCESS_H
#define INIT_JOB_PROCESS_H

#include <sys/types.h>

#include <nih/macros.h>
#include <nih/child.h>
#include <nih/error.h>

#include "process.h"
#include "job_class.h"
#include "job.h"


/**
 * JOB_PROCESS_SCRIPT_FD:
 *
 * The special fd used to pass the script to the shell process, this can be
 * anything from 3-9 (0-2 are stdin/out/err, 10 and above aren't guaranteed
 * by POSIX).
 **/
#define JOB_PROCESS_SCRIPT_FD 9

/**
 * JOB_PROCESS_LOG_REMAP_FROM_CHAR:
 * JOB_PROCESS_LOG_REMAP_TO_CHAR:
 *
 * All logs are written to a single directory so any jobs containing
 * slashes must be remapped.
 **/
#ifndef JOB_PROCESS_LOG_REMAP_FROM_CHAR
#define JOB_PROCESS_LOG_REMAP_FROM_CHAR  '/'
#endif
#ifndef JOB_PROCESS_LOG_REMAP_TO_CHAR
#define JOB_PROCESS_LOG_REMAP_TO_CHAR    '_'
#endif

/**
 * JOB_PROCESS_LOG_FILE_EXT:
 *
 * Extension for log files.
 **/
#ifndef JOB_PROCESS_LOG_FILE_EXT
#define JOB_PROCESS_LOG_FILE_EXT ".log"
#endif

/**
 * JobProcessErrorType:
 *
 * These constants represent the different steps of process spawning that
 * can produce an error.
 **/
typedef enum job_process_error_type {
	JOB_PROCESS_ERROR_DUP,
	JOB_PROCESS_ERROR_CONSOLE,
	JOB_PROCESS_ERROR_RLIMIT,
	JOB_PROCESS_ERROR_PRIORITY,
	JOB_PROCESS_ERROR_OOM_ADJ,
	JOB_PROCESS_ERROR_CHROOT,
	JOB_PROCESS_ERROR_CHDIR,
	JOB_PROCESS_ERROR_PTRACE,
	JOB_PROCESS_ERROR_EXEC,
	JOB_PROCESS_ERROR_GETPWNAM,
	JOB_PROCESS_ERROR_GETGRNAM,
	JOB_PROCESS_ERROR_GETPWUID,
	JOB_PROCESS_ERROR_BAD_SETUID,
	JOB_PROCESS_ERROR_BAD_SETGID,
	JOB_PROCESS_ERROR_SETUID,
	JOB_PROCESS_ERROR_SETGID,
	JOB_PROCESS_ERROR_CHOWN,
	JOB_PROCESS_ERROR_UNLOCKPT,
	JOB_PROCESS_ERROR_GRANTPT,
	JOB_PROCESS_ERROR_PTSNAME,
	JOB_PROCESS_ERROR_OPENPT_SLAVE,
	JOB_PROCESS_ERROR_SIGNAL,
	JOB_PROCESS_ERROR_ALLOC,
	JOB_PROCESS_ERROR_INITGROUPS,
	JOB_PROCESS_ERROR_GETGRGID,
	JOB_PROCESS_ERROR_SECURITY,
	JOB_PROCESS_ERROR_CGROUP_MGR_CONNECT,
	JOB_PROCESS_ERROR_CGROUP_SETUP,
	JOB_PROCESS_ERROR_CGROUP_ENTER,
	JOB_PROCESS_ERROR_CGROUP_CLEAR
} JobProcessErrorType;

/**
 * JobProcessError:
 * @error: ordinary NihError,
 * @type: specific error,
 * @arg: relevant argument to @type,
 * @errnum: system error number.
 *
 * This structure builds on NihError to include additional fields useful
 * for an error generated by spawning a process.  @error includes the single
 * error number and human-readable message which are sufficient for many
 * purposes.
 *
 * @type indicates which step of the spawning process failed, @arg is any
 * information relevant to @type (such as the resource limit that could not
 * be set) and @errnum is the actual system error number.
 *
 * If you receive a JOB_PROCESS_ERROR, the returned NihError structure is
 * actually this structure and can be cast to get the additional fields.
 **/
typedef struct job_process_error {
	NihError            error;
	JobProcessErrorType type;
	int                 arg;
	int                 errnum;
} JobProcessError;


/**
 * JobProcessErrorHandler:
 *
 * @job: job,
 * @state: required state job is attempting to achieve,
 * @process: job process that failed.
 *
 * Function that is called when a job process @process fails to start.
 **/
typedef void (*JobProcessErrorHandler) (Job *job, JobState state, ProcessType process);

NIH_BEGIN_EXTERN

void   job_process_start      (Job *job, ProcessType process);
void   job_process_run_bottom (JobProcessData *handler_data);

void   job_process_child_reader (JobProcessData *handler_data, NihIo *io,
		const char *buf, size_t len);

void   job_process_close_handler (JobProcessData *handler_data, NihIo *io);

pid_t  job_process_spawn   (Job *job, char * const argv[],
			    char * const *env, int trace, int script_fd,
			    ProcessType   process)
	__attribute__ ((warn_unused_result));

pid_t  job_process_spawn_with_fd   (Job *job, char * const argv[],
			    char * const *env, int trace, int script_fd,
			    ProcessType process, int *job_process_fd)
	__attribute__ ((warn_unused_result));


void   job_process_kill    (Job *job, ProcessType process);

void   job_process_handler (void *ptr, pid_t pid,
			    NihChildEvents event, int status);

Job   *job_process_find     (pid_t pid, ProcessType *process);

char  *job_process_log_path (Job *job, int user_job)
	__attribute__ ((warn_unused_result));

void   job_process_set_kill_timer (Job          *job,
				   ProcessType   process,
				   time_t        timeout);

void   job_process_adj_kill_timer  (Job *job, time_t due);

int    job_process_jobs_running (void);

void   job_process_stop_all (void);

JobProcessData *
job_process_data_new (void *parent, Job *job, ProcessType process, int job_process_fd)
	__attribute__ ((warn_unused_result));

json_object *
job_process_data_serialise (const Job *job, const JobProcessData *handler_data)
	__attribute__ ((warn_unused_result));

JobProcessData *
job_process_data_deserialise (void *parent, Job *job, json_object *json)
	__attribute__ ((warn_unused_result));

void job_process_error_handler (const char *buf, size_t len);

void job_process_error_abort     (int fd, JobProcessErrorType type,
					 int arg)
	__attribute__ ((noreturn));

NIH_END_EXTERN

#endif /* INIT_JOB_PROCESS_H */