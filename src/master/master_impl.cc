// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "master_impl.h"
#include <gflags/gflags.h>
#include "master_util.h"
#include <logging.h>

DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_string(master_lock_path);
DECLARE_string(master_path);
DECLARE_string(jobs_store_path);

namespace baidu {
namespace galaxy {

MasterImpl::MasterImpl() : nexus_(NULL) {
    nexus_ = new InsSDK(FLAGS_nexus_servers);
}

MasterImpl::~MasterImpl() {
    delete nexus_;
}

static void OnMasterLockChange(const ::galaxy::ins::sdk::WatchParam& param, 
                               ::galaxy::ins::sdk::SDKError error) {
    MasterImpl* master = static_cast<MasterImpl*>(param.context);
    master->OnLockChange(param.value);
}

static void OnMasterSessionTimeout(void* ctx) {
    MasterImpl* master = static_cast<MasterImpl*>(ctx);
    master->OnSessionTimeout();
}

void MasterImpl::OnLockChange(std::string lock_session_id) {
    std::string self_session_id = nexus_->GetSessionID();
    if (self_session_id != lock_session_id) {
        LOG(FATAL, "master lost lock , die.");
        abort();
    }
}

void MasterImpl::OnSessionTimeout() {
    LOG(FATAL, "master lost session with nexus, die.");
    abort();
}

void MasterImpl::Init() {
    AcquireMasterLock();
    LOG(INFO, "begin to reload job descriptor from nexus");
    ReloadJobInfo();
}

void MasterImpl::ReloadJobInfo() {
    std::string start_key = FLAGS_nexus_root_path + FLAGS_jobs_store_path + "/";
    std::string end_key = start_key + "~";
    ::galaxy::ins::sdk::ScanResult* result = nexus_->Scan(start_key, end_key);
    int job_amount = 0;
    while (!result->Done()) {
        assert(result->Error() == ::galaxy::ins::sdk::kOK);
        std::string key = result->Key();
        std::string job_raw_data = result->Value(); 
        JobInfo job_info;
        bool ok = job_info.ParseFromString(job_raw_data);
        if (ok) {
            LOG(INFO, "reload job: %s", job_info.jobid().c_str());
            job_manager_.ReloadJobInfo(job_info);
        } else {
            LOG(WARNING, "faild to parse job_info: %s", key.c_str());
        }
        result->Next();
        job_amount ++;
    }
    LOG(INFO, "reload all job desc finish, total#: %d", job_amount);
}

void MasterImpl::AcquireMasterLock() {
    std::string master_lock = FLAGS_nexus_root_path + FLAGS_master_lock_path;
    ::galaxy::ins::sdk::SDKError err;
    nexus_->RegisterSessionTimeout(&OnMasterSessionTimeout, this);
    bool ret = nexus_->Lock(master_lock, &err); //whould block until accquired
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    std::string master_endpoint = MasterUtil::SelfEndpoint();
    std::string master_path_key = FLAGS_nexus_root_path + FLAGS_master_path;
    ret = nexus_->Put(master_path_key, master_endpoint, &err);
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    nexus_->Watch(master_lock, &OnMasterLockChange, this, &err);
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    LOG(INFO, "master lock [ok].  %s -> %s", 
        master_path_key.c_str(), master_endpoint.c_str());
}

void MasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::SubmitJobRequest* request,
                           ::baidu::galaxy::SubmitJobResponse* response,
                           ::google::protobuf::Closure* done) {
    const JobDescriptor& job_desc = request->job();
    JobId job_id = MasterUtil::GenerateJobId(job_desc);

    std::string job_raw_data;
    JobInfo job_info;
    job_info.mutable_desc()->CopyFrom(job_desc);
    job_info.set_jobid(job_id);
    job_info.SerializeToString(&job_raw_data);
    job_info.set_state(kJobNormal);
    std::string job_key = FLAGS_nexus_root_path + FLAGS_jobs_store_path 
                          + "/" + job_id;
    ::galaxy::ins::sdk::SDKError err;
    bool put_ok = nexus_->Put(job_key, job_raw_data, &err);
    if (!put_ok) {
        response->set_status(kJobSubmitFail);
        LOG(WARNING, "save job_desc to nexus fail, reason: %s",
            ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
        done->Run();
        return;
    }

    job_manager_.Add(job_id, job_desc);
    response->set_jobid(job_id);
    done->Run();
}

void MasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::UpdateJobRequest*,
                           ::baidu::galaxy::UpdateJobResponse*,
                           ::google::protobuf::Closure* done) {
    controller->SetFailed("Method UpdateJob() not implemented.");
    done->Run();
}

void MasterImpl::SuspendJob(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::SuspendJobRequest* request,
                            ::baidu::galaxy::SuspendJobResponse* response,
                            ::google::protobuf::Closure* done) {
    response->set_status(job_manager_.Suspend(request->jobid()));
    done->Run();
}

void MasterImpl::ResumeJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::ResumeJobRequest* request,
                           ::baidu::galaxy::ResumeJobResponse* response,
                           ::google::protobuf::Closure* done) {
    response->set_status(job_manager_.Resume(request->jobid()));
    done->Run();
}

void MasterImpl::TerminateJob(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::TerminateJobRequest*,
                              ::baidu::galaxy::TerminateJobResponse*,
                              ::google::protobuf::Closure* done) {
    controller->SetFailed("Method TerminateJob() not implemented.");
    done->Run();
}

void MasterImpl::ShowJob(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::ShowJobRequest* request,
                         ::baidu::galaxy::ShowJobResponse* response,
                         ::google::protobuf::Closure* done) {
    for (int32_t i = 0; i < request->jobsid_size(); i++) {
        const JobId& jobid = request->jobsid(i);
        if (kOk != job_manager_.GetJobInfo(jobid, response->mutable_jobs()->Add())) {
            response->mutable_jobs()->RemoveLast();
        }
    }
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::ListJobs(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::ListJobsRequest* request,
                          ::baidu::galaxy::ListJobsResponse* response,
                          ::google::protobuf::Closure* done) {
    job_manager_.GetJobsOverview(response->mutable_jobs());
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::HeartBeat(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::HeartBeatRequest* request,
                          ::baidu::galaxy::HeartBeatResponse*,
                          ::google::protobuf::Closure* done) {
    std::string agent_addr = request->endpoint();
    job_manager_.KeepAlive(agent_addr);
    done->Run();
}

void MasterImpl::GetPendingJobs(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::GetPendingJobsRequest*,
                                ::baidu::galaxy::GetPendingJobsResponse* response,
                                ::google::protobuf::Closure* done) {
    job_manager_.GetPendingPods(response->mutable_scale_up_jobs());
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::GetResourceSnapshot(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::GetResourceSnapshotRequest* request,
                         ::baidu::galaxy::GetResourceSnapshotResponse* response,
                         ::google::protobuf::Closure* done) {
    job_manager_.GetAliveAgentsInfo(response->mutable_agents());
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::Propose(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::ProposeRequest* request,
                         ::baidu::galaxy::ProposeResponse* response,
                         ::google::protobuf::Closure* done) {
    for (int i = 0; i < request->schedule_size(); i++) {
        const ScheduleInfo& sche_info = request->schedule(i);
        response->set_status(job_manager_.Propose(sche_info));
    }
    done->Run();
    job_manager_.DeployPod();
}

void MasterImpl::ListAgents(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::ListAgentsRequest* request,
                            ::baidu::galaxy::ListAgentsResponse* response,
                            ::google::protobuf::Closure* done) {
    job_manager_.GetAgentsInfo(response->mutable_agents());
    response->set_status(kOk);
    done->Run();
}

}
}
