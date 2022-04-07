namespace web_ui.model
{
    public class LedState
    {
        public int LedNumber { get; set; }

        public LedAction State { get; set; }

        public string? DateChange { get; set; }
    }
}