using web_ui.model;

namespace web_ui.Services
{
    public class Backend : IBackend
    {
        const string report = "report.csv";
        private HttpClient _http { get; }

        public Backend(HttpClient http)
        {
            _http = http;

        }

        public Task<List<LedState>> GetLedsState()
        {
            throw new NotImplementedException();
        }

        public async Task<List<Report>> GetReport()
        {
            var result = new List<Report>();

            var httpResult = await _http.GetStringAsync(report);

            Console.WriteLine(httpResult);

            return result;
        }

        public Task<int> GetServoAngle()
        {
            throw new NotImplementedException();
        }

        public Task MoveServo(int angle)
        {
            throw new NotImplementedException();
        }

        public Task StartNewProcess()
        {
            throw new NotImplementedException();
        }

        public Task SwitchLedOff(int ledNumber)
        {
            throw new NotImplementedException();
        }
    }
}