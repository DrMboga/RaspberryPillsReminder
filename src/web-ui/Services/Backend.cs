using web_ui.helpers;
using web_ui.model;

namespace web_ui.Services
{
    public class Backend : IBackend
    {
        const string report = "report.csv";
        const string ledsState = "leds-state.csv";
        const string servoState = "servo-state.csv";
        const string startCommand = "start";
        const string ledOffCommand = "led/{0}/green/off";
        const string setServoCommand = "servo/{0}";

        private HttpClient _http { get; }

        public Backend(HttpClient http)
        {
            _http = http;

        }

        public async Task<List<LedState>> GetLedsState()
        {
            var httpResult = await _http.GetStringAsync(ledsState);

            return httpResult.ParseCsvToCollection<LedState>();
        }

        public async Task<List<Report>> GetReport()
        {
            var httpResult = await _http.GetStringAsync(report);

            return httpResult.ParseCsvToCollection<Report>();
        }

        public async Task<int> GetServoAngle()
        {
            var httpResult = await _http.GetStringAsync(servoState);
            return int.Parse(httpResult);

        }

        public Task MoveServo(int angle)
        {
            var command = string.Format(setServoCommand, angle);
            return _http.PostAsync(command, null);
        }

        public Task StartNewProcess()
        {
            return _http.PostAsync(startCommand, null);
        }

        public Task SwitchLedOff(int ledNumber)
        {
            var command = string.Format(ledOffCommand, ledNumber);
            return _http.PostAsync(command, null);
        }
    }
}