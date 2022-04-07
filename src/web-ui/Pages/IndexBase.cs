using Microsoft.AspNetCore.Components;
using web_ui.model;
using web_ui.Services;

namespace web_ui.Pages
{
    public class IndexBase : ComponentBase
    {
        [Inject]
        public IBackend BackendService { get; set; }

        public List<Report>? Report { get; set; }
        public List<LedState>? Leds { get; set; }

        public int? ServoAngle { get; set; }

        protected override async Task OnInitializedAsync()
        {
            Report = await BackendService.GetReport();
            Report = Report.OrderByDescending(r => r.DateTaken).ToList();
            Leds = await BackendService.GetLedsState();
            Leds = Leds.OrderBy(l => l.LedNumber).ToList();
            ServoAngle = await BackendService.GetServoAngle();
        }
    }
}