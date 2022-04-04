using web_ui.model;

namespace web_ui.Services
{
    public interface IBackend
    {
        Task<List<Report>> GetReport();

        Task<List<LedState>> GetLedsState();

        Task<int> GetServoAngle();

        Task StartNewProcess();

        Task SwitchLedOff(int ledNumber);

        Task MoveServo(int angle);
    }
}