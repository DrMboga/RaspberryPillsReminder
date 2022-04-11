using Microsoft.AspNetCore.Components;
using web_ui.model;
using web_ui.Services;

namespace web_ui.Pages
{
    public class IndexBase : ComponentBase
    {
        [Inject]
        public IBackend? BackendService { get; set; }

        public List<Report>? Report { get; set; }
        public List<LedState>? Leds { get; set; }

        public int? ServoAngle { get; set; }

        protected override async Task OnInitializedAsync()
        {
            if (BackendService == null)
            {
                return;
            }
            Report = await BackendService.GetReport();
            Report = Report.OrderByDescending(r => r.DateTaken).ToList();
            Leds = await BackendService.GetLedsState();
            Leds = Leds.OrderBy(l => l.LedNumber).ToList();
            ServoAngle = await BackendService.GetServoAngle();
        }

        public async Task StartNewProcess()
        {
            if (BackendService != null)
            {
                Console.WriteLine("Start new process");
                await BackendService.StartNewProcess();
            }

        }

        public void OnServoAngleInput(string step)
        {
            ServoAngle = Convert.ToInt32(step);
        }

        public async Task OnServoAngleChanged(string newServoAngle)
        {
            ServoAngle = Convert.ToInt32(newServoAngle);
            if (ServoAngle.HasValue && BackendService != null)
            {
                Console.WriteLine($"New Servo value {ServoAngle}");
                await BackendService.MoveServo(ServoAngle.Value);
            }
        }

        public string WeekDay(int index)
        {
            switch (index)
            {
                case 0:
                    return "Mo";
                case 1:
                    return "Tu";
                case 2:
                    return "We";
                case 3:
                    return "Th";
                case 4:
                    return "Fr";
                case 5:
                    return "Sa";
                case 6:
                    return "Su";
            }
            return string.Empty;
        }

        public string LedClass(LedAction action)
        {
            switch (action)
            {
                case LedAction.GreenOn:
                    return "led-green";
                case LedAction.RedOn:
                    return "led-red-not-blink";
                case LedAction.RedBlink:
                    return "led-red";
                default:
                    return "led-off";
            }
        }

        public async Task SwitchLedOff(int ledNumber)
        {
            if (BackendService != null)
            {
                Console.WriteLine($"Switch off LED {ledNumber}");
                await BackendService.SwitchLedOff(ledNumber);
            }

        }
    }
}