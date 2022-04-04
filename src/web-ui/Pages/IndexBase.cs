using Microsoft.AspNetCore.Components;
using web_ui.Services;

namespace web_ui.Pages
{
    public class IndexBase : ComponentBase
    {
        [Inject]
        public IBackend BackendService { get; set; }

        protected override async Task OnInitializedAsync()
        {
            var report = await BackendService.GetReport();
        }
    }
}