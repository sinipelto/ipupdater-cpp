// Author: Sinipelto (c) 2019
// Dynamic IP address updater against domain API.
// Reads and parses configuration parameters from configuration file.

#include "ip.hh"
#include "curl/curl.h"
#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Option.hpp"
#include "curlpp/Options.hpp"
#include "curlpp/Form.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <chrono>
#include <ctime>
#include <locale>

using namespace std;

using StrMap = map<string, string>;
using StrPair = pair<string, string>;
using StrVec = vector<string>;

enum IpSource { external, router };

string &ProcessPath(string &path);

StrMap *ReadConfiguration(const string &path);

const updater::ip *ReadIpFromFile(const string &path);
const updater::ip *QueryIpFromUrl(const string &url, const IpSource &source);

void UpdateIp(const updater::ip * const ip, const string &url, const string &apikey, const string &apisecret);
void SaveIpToFile(const updater::ip * const ip, const string &path);

StrPair ParseLine(const string &line);
void RemoveSpaces(string &input);
bool ParseBool(const string &input);
StrVec ParseList(string input, const string &delimiter);

void WriteChangeLog(const string &path, const updater::ip * const oldIp, const updater::ip * const newIp);
void WriteLog(const string &message, const bool &use_cerr=false);

const int &Terminate(const int &code) const;

// Separating line for logging
#define WRITE_EXIT WriteLog("---------------------------------------------------------")

#if __WIN32
const char DELIM = '\\';
static string LogPath = "logs\\";
#else
const char DELIM = '/';
static string LogPath = "logs/";
#endif

const string configFile = "updater.conf";
const string lastipFile = "lastip";
const string recordFile = "record.log";

int main(int argc, char **argv)
{
    if (argc <= 0)
    {
        WriteLog("ERROR: No path argument given. Aborting.", true);
        return Terminate(1);
    }

    string basePath = argv[0];
#if __WIN32
    basePath = ProcessPath(ProcessPath(basePath));
#else
    basePath = ProcessPath(basePath);
#endif
    LogPath = basePath + LogPath;

    const string configPath = basePath + configFile;

    StrMap *config;

    try {
        config = ReadConfiguration(configPath);
    } catch (const exception& e) {
        string msg = "";
        msg.append("ERROR: Failed to read configuration: ");
        msg.append(e.what());
        WriteLog(msg, true);
        WRITE_EXIT;
        throw;
    }

    // Read variables from configuration
    const string domain = config->at("domain");
    const string api_key = config->at("api_key");
    const string api_secret = config->at("api_secret");

    StrVec record_list = ParseList(config->at("record_list"), ",");

    const string ip_url = config->at("ip_url");
    const string api_url_base = config->at("api_url");

    const bool use_router_info = ParseBool(config->at("use_router_info"));
    const string router_url = config->at("router_url");
    const string router_username = config->at("router_username");
    const string router_password = config->at("router_password");
    const string router_keyword = config->at("router_keyword");

    // Free configuration map memory
    delete config;

    // Check that there are domain records to process
    if (record_list.size() <= 0)
    {
        WriteLog("WARNING: No records to process. Exiting.", true);

        // Is a warning so terminate normally, status 0
        return Terminate(0);
    }

    // Load last ip from record file
    const string lastIpPath = basePath + lastipFile;
    const updater::ip *local_ip = ReadIpFromFile(lastIpPath);

    // Query current ip
    const updater::ip *remote_ip;

    if (!use_router_info)
    {
        WriteLog("Using external service for fetching ip address..");
        remote_ip = QueryIpFromUrl(ip_url, IpSource::external);
    }
    else {
        WriteLog("Using local router as external ip source..");
        remote_ip = QueryIpFromUrl(router_url, IpSource::router);
    }

    // Compare IPs
    if (*local_ip == *remote_ip) {
        WriteLog("IP not changed, exiting without changes.");

        delete lastIpPath;
        delete local_ip;
        delete remote_ip;

        return Terminate(0);
    }

    WriteLog("IP address changed from last record, updating the new ip..");
    WriteLog("Old IP: " + local_ip->toString());
    WriteLog("New IP: " + remote_ip->toString());

    // Write the record change to a special record log file
    const string * const recordPath = new string(basePath + recordFile);
    WriteChangeLog(recordPath, local_ip, remote_ip);
    delete recordPath;

    delete local_ip;

    for (const string& type : record_list)
    {
        // Create api url
        const string &api_url = api_url_base + "/" + domain + "/records/A/" + type;

        // If changed, update to server with new ip
        UpdateIp(remote_ip, api_url, api_key, api_secret);

        WriteLog(type + " record updated successfully.");
    }

    WriteLog("IP records updated to server successfully.");

    // Write down new ip to file
    SaveIpToFile(remote_ip, lastIpPath);

    delete remote_ip;
    delete lastIpPath;

    WriteLog("Local IP record updated successfully.");

    // Terminate curlpp and exit
    return Terminate(0);
}

string &ProcessPath(string &path)
{
    auto it = path.rbegin();

    // Remove the first slash
    if (*it == DELIM)
    {
        path.pop_back();
        it++;
    }

    // Keep popping until next slash appears
    while (*it != DELIM)
    {
        path.pop_back();
        it++;
    }

    return path;
}

bool ParseBool(const string &input)
{
    string parsed = "";

    for (char c : input) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') continue;
        parsed += std::tolower(c);
    }

    if (parsed == "")
    {
        WriteLog("ERROR: Could not parse boolean value for: " + input + "in configuration.", true);
        throw new exception;
    }

    if (parsed == "yes" || parsed == "y" || parsed == "true") return true;

    return false;
}

StrVec ParseList(string input, const string &delimiter)
{
    StrVec options = {};

    size_t pos = 0;

    while ((pos = input.find(delimiter)) != std::string::npos)
    {
        string token = input.substr(0, pos);
        input.erase(0, pos + delimiter.length());

        if (token == "") continue;

        options.push_back(token);
    }

    if (input != "") options.push_back(input);

    return options;
}

const int &Terminate(const int &code) const
{
    curlpp::terminate();
    WRITE_EXIT;
    return code;
}

void WriteLog(const string &message, const bool &use_cerr)
{
    // Set time parse format string
    const char *fileFormat = "%Y-%m-%d";
    const char *stampFormat = "[%Y-%m-%d %H:%M:%S] ";

    // Get current time
    time_t now = time(nullptr);

    ostringstream ostring;

    ostring << std::put_time(std::localtime(&now), fileFormat);

    string logFileName;
    logFileName.append(LogPath);
    logFileName.append(ostring.str());
    logFileName.append(".log");

    ostring.seekp(0);

    ofstream logFile;
    logFile.open(logFileName, ios::app);

    if (logFile.fail())
    {
        // Using cout insted of cerr because new file is created nightly
        cout << "WARNING: Failed to open log file. Trying to create new one.." << endl;
        logFile.open(logFileName);

        if (logFile.fail())
        {
            cerr << "ERROR: Failed to create log file, aborting." << endl;
            throw new exception;
        }
    }

    ostring << put_time(localtime(&now), stampFormat) << message;

    logFile << ostring.str() << endl;
    logFile.close();

    if (use_cerr) {
        cerr << put_time(localtime(&now), stampFormat) << message << endl;
        return;
    }

    cout << put_time(localtime(&now), stampFormat) << message << endl;
}

void WriteChangeLog(const string &path, const updater::ip * const oldIp, const updater::ip * const newIp)
{
    // Set time parse format string
    const char *stampFormat = "[%Y-%m-%d %H:%M:%S] ";

    // Get current time
    time_t now = time(nullptr);

    ostringstream ostring;

    ofstream logFile;
    logFile.open(path, ios::app);

    if (logFile.fail())
    {
        WriteLog("WARNING: Failed to open record file. Trying to create new one..", true);
        logFile.open(path);

        if (logFile.fail())
        {
            WriteLog("ERROR: Failed to create record file, aborting..", true);
            throw new exception;
        }
    }

    ostring << put_time(localtime(&now), stampFormat) << "IP Record changed: from " << oldIp->toString() << " to " << newIp->toString();

    logFile << ostring.str() << endl;
    logFile.close();
}


StrPair ParseLine(const string &line)
{
    pair<string, string> keyvalue;

    // line -> key=value
    for (unsigned i = 0; i < line.size(); i++)
    {
        if (line[i] == '=')
        {
            keyvalue.first = line.substr(0, i); // [sdfjsd=], = excluded

            if ( line[ line.size() - 1 ] == '\n' ) {
                keyvalue.second = line.substr(i+1, line.size()-1); // [sdfsahdds\n], \n excluded
            }

            else {
                keyvalue.second = line.substr(i+1, line.size()); // [sdfsahdds]
            }

            break; // Only first '=' is parsed
        }
    }

    return keyvalue;
}

StrMap &ReadConfiguration(const string &path)
{
    StrMap settings = StrMap();

    ifstream file;
    file.open(path);

    if (file.fail())
    {
        WriteLog("ERROR: Could not open configuration file: " + configFile, true);
        delete path;
        WRITE_EXIT;
        throw new exception;
    }

    string line;
    while (getline(file, line))
    {
        if (line[0] == '#') continue;
        else if (line.size() < 3) continue; // Format: 'a=b' is minimum of 3

        RemoveSpaces(line); // Remove any whitespace from the config line
        auto parsed = ParseLine(line);
        settings->operator[](parsed.first) = parsed.second;
    }

    file.close();

    return settings;
}

const updater::ip *ReadIpFromFile(const string &path)
{
    ifstream file;
    file.open(path);

    if (file.fail())
    {
        WriteLog("WARNING: Failed to open IP record file. Trying to create new one with zero record..", true);

        ofstream out;
        out.open(path);

        if (out.fail()) {
            WriteLog("ERROR: Failed to create ip record. Cannot continue.", true);
            delete path;
            WRITE_EXIT;
            throw new exception;
        }

        out << "0.0.0.0";
        out.close();

        WriteLog("New IP record file written, returning with zero record.");
        return new updater::ip(0, 0, 0, 0);
    }

    string line;
    getline(file, line);
    file.close();

    updater::ip *addr;

    try {
        addr = new updater::ip(line);
    } catch (const exception &e) {
        string msg = "";
        msg.append("ERROR: Error while parsing last ip from file: ");
        msg.append(e.what());
        WriteLog(msg, true);
        WRITE_EXIT;
        throw;
    }

    return addr;
}

void UpdateIp(const updater::ip * const ip, const string &url, const string &apikey, const string &apisecret)
{
    curlpp::Easy handle;

    std::stringstream buffer;

    auto authOpt = curlpp::options::HttpHeader({"Content-Type: application/json", "Authorization: sso-key " + apikey + ":" + apisecret});
    auto dataOpt = curlpp::Options::PostFields("[{\"data\":\"" + ip->toString() + "\"}]");

    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CustomRequest("PUT"));
    handle.setOpt(authOpt);
    handle.setOpt(dataOpt);
    handle.setOpt(curlpp::Options::SslVerifyHost(false));
    handle.setOpt(curlpp::Options::SslVerifyPeer(false));
    handle.setOpt(curlpp::Options::WriteStream(&buffer));
    handle.setOpt(curlpp::Options::FailOnError(true));

    try {
        handle.perform();
    } catch (curlpp::RuntimeError &e) {
        string msg = "";
        msg.append("Error while sending update request: ");
        msg.append(e.what());
        WriteLog(msg, true);
        WRITE_EXIT;
        throw;
    }

    string resp;
    string strBuf;

    // Reading response from buffer
    while (buffer >> strBuf) {
        resp += strBuf + " ";
    }

    // Long response indicates a problem
    if (resp.length() > 10)
    {
        WriteLog("Error in update: " + resp, true);
        delete ip;
        WRITE_EXIT;
        throw new exception;
    }
}

const updater::ip *QueryIpFromUrl(const string &url, const IpSource &source)
{
    if (source == IpSource::router)
    {
        WriteLog("ERROR: Router ip fetching not yet implemented. Please use clean source for fetching ip address. Set use_router to false.", true);
        WRITE_EXIT;
        throw new exception;
    }

    curlpp::Easy handle;

    std::stringstream buffer;

    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::HttpGet(true));
    handle.setOpt(curlpp::Options::SslVerifyHost(false));
    handle.setOpt(curlpp::Options::SslVerifyPeer(false));
    handle.setOpt(curlpp::Options::WriteStream(&buffer));
    handle.setOpt(curlpp::Options::FailOnError(true));

    try {
        handle.perform();
    } catch (curlpp::RuntimeError &e) {
        string msg = "";
        msg.append("Error while sending ip get request: ");
        msg.append(e.what());
        WriteLog(msg, true);
        WRITE_EXIT;
        throw;
    }

    string resp;

    buffer >> resp;

    updater::ip *addr;

    try {
        addr = new updater::ip(resp);
    } catch (const std::exception &e) {
        string msg = "";
        msg.append("ERROR: Error while parsing current remote ip: ");
        msg.append(e.what());
        WriteLog(msg, true);
        WRITE_EXIT;
        throw;
    }

    return addr;
}

void SaveIpToFile(const updater::ip * const ip, const string &path)
{
    ofstream file;
    file.open(*path, ios::out);

    if (file.fail()) {
        WriteLog("ERROR: Failed to open ip record file. Ensure necessary resources and permissions are available for operation.", true);
        delete path;
        WRITE_EXIT;
        throw new exception;
    }

    file << ip->toString();
    file.close();
}

void RemoveSpaces(string &input)
{
    if (input == "") return;

    auto it = input.begin();

    while (it != input.end())
    {
        if (*it == ' ' || *it == '\t' || *it == '\n' || *it == '\r') {
            input.erase(it);
        }

        it++;
    }
}
