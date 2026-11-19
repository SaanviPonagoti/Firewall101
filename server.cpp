#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <direct.h>
  typedef SOCKET sock_t;
  #define CLOSESOCK closesocket
  #define MKDIR(p) _mkdir(p)
  static const sock_t BAD_SOCK = INVALID_SOCKET;
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <unistd.h>
  typedef int sock_t;
  #define CLOSESOCK close
  #define MKDIR(p) mkdir(p, 0755)
  static const sock_t BAD_SOCK = -1;
#endif

using std::string;
using std::vector;


static string SITE_DIR = "..";
static string DATA_FILE = "data/reports.tsv";
static string UPLOAD_DIR = "uploads";
static string ADMIN_KEY = "admin123";
static int PORT = 8080;
static const size_t MAX_BODY = 8 * 1024 * 1024;
static const size_t MAX_FILE = 5 * 1024 * 1024;

static const vector<string> TYPES = {
    "Phishing Email", "Ransomware", "Malware Infection", "Password / Account Compromise",
    "Data Breach", "Lost or Stolen Device", "Denial-of-Service Attack", "Website Defacement",
    "Other / Not sure"};
static const vector<string> SEVERITIES = {"Low", "Medium", "High", "Critical"};
static const vector<string> STATUSES = {"Submitted", "Under Review", "Contained", "Resolved", "Closed"};

static bool contains(const vector<string>& v, const string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}


struct Report {
    string id, submittedAt, name, email, dept, type, severity, datetime, system, description, attachment, status;
};

static string tsvEscape(const string& s) {
    string o;
    for (char c : s) {
        if (c == '\\') o += "\\\\";
        else if (c == '\t') o += "\\t";
        else if (c == '\n') o += "\\n";
        else if (c == '\r') {}
        else o += c;
    }
    return o;
}
static string tsvUnescape(const string& s) {
    string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            o += (n == 't') ? '\t' : (n == 'n') ? '\n' : n;
        } else o += s[i];
    }
    return o;
}

static vector<Report> loadReports() {
    vector<Report> out;
    std::ifstream f(DATA_FILE);
    string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p;
        std::stringstream ss(line);
        string cell;
        while (std::getline(ss, cell, '\t')) p.push_back(tsvUnescape(cell));
        if (p.size() < 12) continue;
        out.push_back({p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]});
    }
    return out;
}
static void saveReports(const vector<Report>& v) {
    string tmp = DATA_FILE + ".tmp";
    {
        std::ofstream f(tmp, std::ios::trunc);
        for (const auto& r : v) {
            f << tsvEscape(r.id) << '\t' << tsvEscape(r.submittedAt) << '\t' << tsvEscape(r.name) << '\t'
              << tsvEscape(r.email) << '\t' << tsvEscape(r.dept) << '\t' << tsvEscape(r.type) << '\t'
              << tsvEscape(r.severity) << '\t' << tsvEscape(r.datetime) << '\t' << tsvEscape(r.system) << '\t'
              << tsvEscape(r.description) << '\t' << tsvEscape(r.attachment) << '\t' << tsvEscape(r.status) << '\n';
        }
    }
    std::remove(DATA_FILE.c_str());
    std::rename(tmp.c_str(), DATA_FILE.c_str());
}


static string jsonEscape(const string& s) {
    string o;
    for (unsigned char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (c < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", c); o += b; }
                else o += (char)c;
        }
    }
    return o;
}
static string reportToJson(const Report& r) {
    return "{\"id\":\"" + jsonEscape(r.id) + "\",\"submittedAt\":\"" + jsonEscape(r.submittedAt) +
           "\",\"name\":\"" + jsonEscape(r.name) + "\",\"email\":\"" + jsonEscape(r.email) +
           "\",\"dept\":\"" + jsonEscape(r.dept) + "\",\"type\":\"" + jsonEscape(r.type) +
           "\",\"severity\":\"" + jsonEscape(r.severity) + "\",\"datetime\":\"" + jsonEscape(r.datetime) +
           "\",\"system\":\"" + jsonEscape(r.system) + "\",\"description\":\"" + jsonEscape(r.description) +
           "\",\"attachment\":\"" + jsonEscape(r.attachment) + "\",\"status\":\"" + jsonEscape(r.status) + "\"}";
}
static string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}
static string lower(string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
static string urlDecode(const string& s) {
    string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') o += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && std::isxdigit((unsigned char)s[i + 1]) && std::isxdigit((unsigned char)s[i + 2])) {
            o += (char)std::stoi(s.substr(i + 1, 2), nullptr, 16);
            i += 2;
        } else o += s[i];
    }
    return o;
}
static std::map<string, string> parseUrlEncoded(const string& body) {
    std::map<string, string> m;
    size_t pos = 0;
    while (pos <= body.size()) {
        size_t amp = body.find('&', pos);
        if (amp == string::npos) amp = body.size();
        string pair = body.substr(pos, amp - pos);
        size_t eq = pair.find('=');
        if (eq != string::npos) m[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
        pos = amp + 1;
    }
    return m;
}

struct Upload { string filename, data; bool present = false; };


static void parseMultipart(const string& body, const string& boundary,
                           std::map<string, string>& fields, Upload& up) {
    string delim = "--" + boundary;
    size_t pos = body.find(delim);
    while (pos != string::npos) {
        pos += delim.size();
        if (body.compare(pos, 2, "--") == 0) break;
        if (body.compare(pos, 2, "\r\n") == 0) pos += 2;
        size_t hdrEnd = body.find("\r\n\r\n", pos);
        if (hdrEnd == string::npos) break;
        string headers = body.substr(pos, hdrEnd - pos);
        size_t dataStart = hdrEnd + 4;
        size_t next = body.find("\r\n" + delim, dataStart);
        if (next == string::npos) break;
        string data = body.substr(dataStart, next - dataStart);

        auto getAttr = [&](const string& key) -> string {
            size_t k = headers.find(key + "=\"");
            if (k == string::npos) return "";
            k += key.size() + 2;
            size_t e = headers.find('"', k);
            return e == string::npos ? "" : headers.substr(k, e - k);
        };
        string name = getAttr("name");
        string fname = getAttr("filename");
        if (!fname.empty()) {
            if (!data.empty()) { up.filename = fname; up.data = data; up.present = true; }
        } else if (!name.empty()) {
            fields[name] = data;
        }
        pos = next + 2;
    }
}

static string sanitizeFilename(const string& in) {
    string base = in;
    size_t slash = base.find_last_of("/\\");
    if (slash != string::npos) base = base.substr(slash + 1);
    string o;
    for (char c : base) o += (std::isalnum((unsigned char)c) || c == '.' || c == '-' || c == '_') ? c : '_';
    if (o.size() > 80) o = o.substr(o.size() - 80);
    return o;
}
static bool allowedExtension(const string& name) {
    size_t dot = name.find_last_of('.');
    if (dot == string::npos) return false;
    string ext = lower(name.substr(dot + 1));
    static const vector<string> ok = {"png", "jpg", "jpeg", "gif", "pdf", "txt", "eml"};
    return contains(ok, ext);
}
static bool validEmail(const string& e) {
    size_t at = e.find('@');
    if (at == string::npos || at == 0 || e.find('@', at + 1) != string::npos) return false;
    size_t dot = e.find('.', at);
    return dot != string::npos && dot > at + 1 && dot + 1 < e.size() && e.find(' ') == string::npos;
}
static string nowIso() {
    time_t t = time(nullptr);
    char b[32];
    std::strftime(b, sizeof b, "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));
    return string(b) + "Z";
}
static string yearNow() { return nowIso().substr(0, 4); }

// ---------- HTTP ----------
struct Request {
    string method, path, body;
    std::map<string, string> headers;
};
struct Response {
    int status = 200;
    string type = "application/json";
    string body;
};

static const char* statusText(int s) {
    switch (s) {
        case 200: return "OK"; case 201: return "Created"; case 204: return "No Content";
        case 400: return "Bad Request"; case 401: return "Unauthorized"; case 403: return "Forbidden";
        case 404: return "Not Found"; case 405: return "Method Not Allowed"; case 413: return "Payload Too Large";
        default: return "Internal Server Error";
    }
}
static Response jsonError(int status, const string& msg) {
    Response r; r.status = status; r.body = "{\"error\":\"" + jsonEscape(msg) + "\"}"; return r;
}

static bool sendAll(sock_t s, const string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        int n = (int)send(s, data.data() + sent, (int)(data.size() - sent), 0);
        if (n <= 0) return false;
        sent += (size_t)n;
    }
    return true;
}
static void sendResponse(sock_t s, const Response& r) {
    std::ostringstream h;
    h << "HTTP/1.1 " << r.status << " " << statusText(r.status) << "\r\n"
      << "Content-Type: " << r.type << "\r\n"
      << "Content-Length: " << r.body.size() << "\r\n"
      << "X-Content-Type-Options: nosniff\r\n"
      << "Cache-Control: no-store\r\n"
      << "Connection: close\r\n\r\n";
    sendAll(s, h.str());
    sendAll(s, r.body);
}

static bool readRequest(sock_t s, Request& req, int& errStatus) {
    string buf;
    char tmp[4096];
    size_t headerEnd = string::npos;
    while (headerEnd == string::npos) {
        int n = (int)recv(s, tmp, sizeof tmp, 0);
        if (n <= 0) return false;
        buf.append(tmp, (size_t)n);
        headerEnd = buf.find("\r\n\r\n");
        if (buf.size() > 64 * 1024 && headerEnd == string::npos) { errStatus = 400; return false; }
    }
    std::istringstream hs(buf.substr(0, headerEnd));
    string line;
    std::getline(hs, line);
    std::istringstream rl(line);
    string target, version;
    rl >> req.method >> target >> version;
    size_t q = target.find('?');
    req.path = urlDecode(q == string::npos ? target : target.substr(0, q));
    while (std::getline(hs, line)) {
        size_t c = line.find(':');
        if (c == string::npos) continue;
        req.headers[lower(trim(line.substr(0, c)))] = trim(line.substr(c + 1));
    }
    size_t contentLength = 0;
    if (req.headers.count("content-length")) contentLength = (size_t)std::strtoull(req.headers["content-length"].c_str(), nullptr, 10);
    if (contentLength > MAX_BODY) { errStatus = 413; return false; }
    req.body = buf.substr(headerEnd + 4);
    while (req.body.size() < contentLength) {
        int n = (int)recv(s, tmp, sizeof tmp, 0);
        if (n <= 0) return false;
        req.body.append(tmp, (size_t)n);
    }
    req.body.resize(contentLength);
    return true;
}

// ---------- API handlers ----------
static bool isAdmin(const Request& req) {
    auto it = req.headers.find("x-admin-key");
    return it != req.headers.end() && it->second == ADMIN_KEY;
}

static Response handleCreate(const Request& req) {
    std::map<string, string> f;
    Upload up;
    string ct = req.headers.count("content-type") ? req.headers.at("content-type") : "";
    if (lower(ct).find("multipart/form-data") != string::npos) {
        size_t b = ct.find("boundary=");
        if (b == string::npos) return jsonError(400, "Missing multipart boundary.");
        string boundary = ct.substr(b + 9);
        if (!boundary.empty() && boundary[0] == '"') boundary = boundary.substr(1, boundary.find('"', 1) - 1);
        parseMultipart(req.body, boundary, f, up);
    } else {
        f = parseUrlEncoded(req.body);
    }

    Report r;
    r.name = trim(f["name"]); r.email = trim(f["email"]); r.dept = trim(f["dept"]);
    r.type = trim(f["type"]); r.severity = trim(f["severity"]); r.datetime = trim(f["datetime"]);
    r.system = trim(f["system"]); r.description = trim(f["description"]);

    if (r.name.size() < 2 || r.name.size() > 100) return jsonError(400, "Please enter your full name.");
    if (!validEmail(r.email) || r.email.size() > 150) return jsonError(400, "Enter a valid email address.");
    if (r.dept.size() < 2 || r.dept.size() > 100) return jsonError(400, "Enter your department or role.");
    if (!contains(TYPES, r.type)) return jsonError(400, "Select a valid incident type.");
    if (!contains(SEVERITIES, r.severity)) return jsonError(400, "Select a valid severity.");
    if (r.datetime.size() < 10 || r.datetime.size() > 30) return jsonError(400, "Enter a valid date and time.");
    if (r.system.size() < 2 || r.system.size() > 150) return jsonError(400, "Name the affected system or device.");
    if (r.description.size() < 20 || r.description.size() > 4000) return jsonError(400, "Description must be 20 to 4000 characters.");
    if (up.present) {
        if (up.data.size() > MAX_FILE) return jsonError(400, "File is larger than 5 MB.");
        if (!allowedExtension(up.filename)) return jsonError(400, "File type not allowed (png, jpg, gif, pdf, txt, eml).");
    }

    vector<Report> all = loadReports();
    char num[16];
    std::snprintf(num, sizeof num, "%04d", (int)all.size() + 1);
    r.id = "IR-" + yearNow() + "-" + num;
    r.submittedAt = nowIso();
    r.status = "Submitted";
    if (up.present) {
        string saved = r.id + "_" + sanitizeFilename(up.filename);
        std::ofstream out(UPLOAD_DIR + "/" + saved, std::ios::binary);
        out.write(up.data.data(), (std::streamsize)up.data.size());
        r.attachment = saved;
    }
    all.push_back(r);
    saveReports(all);
    std::cout << "[" << r.submittedAt << "] new report " << r.id << " (" << r.type << ", " << r.severity << ")\n";

    Response res; res.status = 201;
    res.body = "{\"id\":\"" + r.id + "\",\"status\":\"Submitted\"}";
    return res;
}

static Response handleList(const Request& req) {
    if (!isAdmin(req)) return jsonError(401, "Admin key required.");
    vector<Report> all = loadReports();
    std::reverse(all.begin(), all.end());   // newest first
    Response r;
    r.body = "[";
    for (size_t i = 0; i < all.size(); i++) r.body += (i ? "," : "") + reportToJson(all[i]);
    r.body += "]";
    return r;
}

static Response handleStatus(const Request& req, const string& id) {
    if (!isAdmin(req)) return jsonError(401, "Admin key required.");
    string status = parseUrlEncoded(req.body)["status"];
    if (!contains(STATUSES, status)) return jsonError(400, "Invalid status.");
    vector<Report> all = loadReports();
    for (auto& r : all) {
        if (r.id == id) {
            r.status = status;
            saveReports(all);
            std::cout << "[" << nowIso() << "] " << id << " -> " << status << "\n";
            Response res; res.body = "{\"id\":\"" + jsonEscape(id) + "\",\"status\":\"" + jsonEscape(status) + "\"}";
            return res;
        }
    }
    return jsonError(404, "Report not found.");
}

// ---------- static files ----------
static string mimeFor(const string& path) {
    size_t dot = path.find_last_of('.');
    string ext = dot == string::npos ? "" : lower(path.substr(dot + 1));
    if (ext == "html") return "text/html; charset=utf-8";
    if (ext == "css") return "text/css; charset=utf-8";
    if (ext == "js") return "application/javascript; charset=utf-8";
    if (ext == "txt") return "text/plain; charset=utf-8";
    if (ext == "png") return "image/png";
    if (ext == "ico") return "image/x-icon";
    return "";
}
static Response handleStatic(const Request& req) {
    string p = req.path;
    if (p == "/" || p.empty()) p = "/index.html";
    if (p.find("..") != string::npos || p.find('\\') != string::npos || p[0] != '/') return jsonError(403, "Forbidden.");
    // only the website's own files: never backend code, data or uploads
    if (p.rfind("/backend", 0) == 0 || p.rfind("/data", 0) == 0 || p.rfind("/uploads", 0) == 0) return jsonError(403, "Forbidden.");
    string mime = mimeFor(p);
    if (mime.empty()) return jsonError(403, "Forbidden.");
    std::ifstream f(SITE_DIR + p, std::ios::binary);
    if (!f) return jsonError(404, "Not found.");
    std::ostringstream ss; ss << f.rdbuf();
    Response r; r.type = mime; r.body = ss.str();
    return r;
}

static Response route(const Request& req) {
    if (req.path == "/api/reports") {
        if (req.method == "POST") return handleCreate(req);
        if (req.method == "GET") return handleList(req);
        return jsonError(405, "Method not allowed.");
    }
    const string pre = "/api/reports/", suf = "/status";
    if (req.path.rfind(pre, 0) == 0 && req.path.size() > pre.size() + suf.size() &&
        req.path.compare(req.path.size() - suf.size(), suf.size(), suf) == 0) {
        if (req.method != "POST") return jsonError(405, "Method not allowed.");
        return handleStatus(req, req.path.substr(pre.size(), req.path.size() - pre.size() - suf.size()));
    }
    if (req.path.rfind("/api/", 0) == 0) return jsonError(404, "Unknown API route.");
    if (req.method != "GET") return jsonError(405, "Method not allowed.");
    return handleStatic(req);
}

// ---------- main ----------
int main(int argc, char** argv) {
    if (const char* k = std::getenv("ADMIN_KEY")) ADMIN_KEY = k;
    if (const char* p = std::getenv("PORT")) PORT = std::atoi(p);
    if (argc > 1) SITE_DIR = argv[1];   // e.g. ./server ..

    MKDIR("data"); MKDIR("uploads");

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { std::cerr << "WSAStartup failed\n"; return 1; }
#endif
    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == BAD_SOCK) { std::cerr << "Cannot create socket\n"; return 1; }
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof yes);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // local only: safer for a demo
    addr.sin_port = htons((unsigned short)PORT);
    if (bind(srv, (sockaddr*)&addr, sizeof addr) < 0) { std::cerr << "Cannot bind port " << PORT << " (already in use?)\n"; return 1; }
    listen(srv, 16);

    std::cout << "Firewall101 backend running at http://localhost:" << PORT << "\n"
              << "Site folder : " << SITE_DIR << "\nData file   : " << DATA_FILE
              << "\nAdmin key   : " << (ADMIN_KEY == "admin123" ? "admin123 (default, set ADMIN_KEY to change)" : "(from ADMIN_KEY)")
              << "\nPress Ctrl+C to stop.\n";

    for (;;) {
        sock_t c = accept(srv, nullptr, nullptr);
        if (c == BAD_SOCK) continue;
#ifdef _WIN32
        DWORD tv = 5000;
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
#else
        timeval tv{5, 0};
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
#endif
        Request req;
        int err = 400;
        if (readRequest(c, req, err)) sendResponse(c, route(req));
        else if (err != 400 || !req.method.empty()) sendResponse(c, jsonError(err, "Bad request."));
        CLOSESOCK(c);
    }
}
