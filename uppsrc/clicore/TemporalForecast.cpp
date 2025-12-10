#include "TemporalForecast.h"

Vector<ForecastPoint> TemporalForecast::ForecastLifecycle(
    const Vector<PhaseSample>& history,
    int horizon
) const {
    Vector<ForecastPoint> forecast;
    
    if (history.GetCount() < 2) {
        // Not enough history to make a forecast, return empty forecast
        return forecast;
    }
    
    // Calculate entropy trend based on recent history
    double entropy_slope = 0.0;
    if (history.GetCount() >= 2) {
        int n = history.GetCount();
        double t_diff = history[n-1].t - history[0].t;
        if (t_diff != 0) {
            entropy_slope = (history[n-1].entropy - history[0].entropy) / t_diff;
        }
    }
    
    // Calculate average entropy change for confidence
    double avg_abs_entropy_change = 0.0;
    for (int i = 1; i < history.GetCount(); i++) {
        avg_abs_entropy_change += abs(history[i].entropy - history[i-1].entropy);
    }
    if (history.GetCount() > 1) {
        avg_abs_entropy_change /= (history.GetCount() - 1);
    }
    
    // Project forward based on trend, with decreasing confidence
    int last_t = history.Top().t;
    double last_entropy = history.Top().entropy;
    String last_phase = history.Top().phase;
    
    for (int i = 1; i <= horizon; i++) {
        ForecastPoint point;
        point.t = last_t + i;
        
        // Predict phase based on entropy trend and possible phase transitions
        if (entropy_slope > 0.02) {
            point.predicted_phase = "complexity_growth";
        } else if (entropy_slope < -0.02) {
            point.predicted_phase = "stabilization";
        } else {
            point.predicted_phase = last_phase;
        }
        
        // Extrapolate entropy value
        point.predicted_entropy = last_entropy + (entropy_slope * i);
        
        // Confidence decreases over time (higher uncertainty in distant future)
        point.confidence = max(0.0, 1.0 - (0.1 * i) - (0.05 * avg_abs_entropy_change));
        
        forecast.Add(point);
    }
    
    return forecast;
}

RiskProfile TemporalForecast::ComputeRiskProfile(
    const Vector<PhaseSample>& history,
    const ReleaseCadence& cadence
) const {
    RiskProfile profile;
    
    // Initialize all risk values
    profile.long_term_risk = 0.0;
    profile.volatility_risk = 0.0;
    profile.schedule_risk = 0.0;
    profile.architectural_risk = 0.0;
    
    if (history.GetCount() < 2) {
        // Set default values if not enough history
        profile.long_term_risk = 0.5;
        profile.volatility_risk = 0.5;
        profile.schedule_risk = 0.5;
        profile.architectural_risk = 0.5;
        return profile;
    }
    
    // Calculate volatility risk based on entropy fluctuations
    double entropy_variance = 0.0;
    double entropy_mean = 0.0;
    
    // Calculate mean entropy
    for (const auto& sample : history) {
        entropy_mean += sample.entropy;
    }
    entropy_mean /= history.GetCount();
    
    // Calculate variance
    for (const auto& sample : history) {
        entropy_variance += (sample.entropy - entropy_mean) * (sample.entropy - entropy_mean);
    }
    entropy_variance /= history.GetCount();
    double entropy_std_dev = sqrt(entropy_variance);
    
    // Normalize volatility risk (0-1 scale)
    profile.volatility_risk = min(1.0, entropy_std_dev * 2.0);
    
    // Calculate schedule risk based on release cadence irregularity
    profile.schedule_risk = cadence.irregularity_score;
    if (profile.schedule_risk > 1.0) profile.schedule_risk = 1.0;
    
    // Calculate architectural risk based on entropy trend
    if (history.GetCount() >= 2) {
        double entropy_trend = 
            (history.Top().entropy - history[0].entropy) / 
            (history.Top().t - history[0].t);
        profile.architectural_risk = max(0.0, min(1.0, abs(entropy_trend) * 5.0));
    }
    
    // Calculate long-term risk as weighted combination of all risks
    profile.long_term_risk = 
        0.3 * profile.volatility_risk + 
        0.4 * profile.schedule_risk + 
        0.3 * profile.architectural_risk;
    
    // Add possible shock scenarios based on identified risks
    profile.possible_shocks.Clear();
    
    // Developer churn scenario (high if volatility is high)
    if (profile.volatility_risk > 0.7) {
        ShockScenario churn;
        churn.type = "developer_churn";
        churn.severity = profile.volatility_risk;
        churn.probability = min(1.0, profile.volatility_risk * 0.8);
        profile.possible_shocks.Add(churn);
    }
    
    // API break scenario (high if architectural risk is high)
    if (profile.architectural_risk > 0.6) {
        ShockScenario api_break;
        api_break.type = "api_break";
        api_break.severity = profile.architectural_risk;
        api_break.probability = min(1.0, profile.architectural_risk * 0.7);
        profile.possible_shocks.Add(api_break);
    }
    
    // Mass refactor scenario (high if entropy is growing rapidly)
    double entropy_growth_rate = 0.0;
    if (history.GetCount() >= 2) {
        int time_span = history.Top().t - history[0].t;
        if (time_span > 0) {
            entropy_growth_rate = (history.Top().entropy - history[0].entropy) / time_span;
        }
    }
    
    if (entropy_growth_rate > 0.05) {
        ShockScenario refactor;
        refactor.type = "mass_refactor";
        refactor.severity = min(1.0, entropy_growth_rate * 10.0);
        refactor.probability = min(1.0, entropy_growth_rate * 8.0);
        profile.possible_shocks.Add(refactor);
    }
    
    return profile;
}

ShockScenario TemporalForecast::SimulateShock(
    const Vector<PhaseSample>& history,
    const String& type
) const {
    ShockScenario scenario;
    scenario.type = type;
    scenario.severity = 0.0;
    scenario.probability = 0.0;
    
    if (history.GetCount() < 1) {
        return scenario;
    }
    
    // Use different logic based on shock type
    if (type == "developer_churn") {
        // Severity based on entropy volatility
        double entropy_variance = 0.0;
        double entropy_mean = 0.0;
        
        // Calculate mean entropy
        for (const auto& sample : history) {
            entropy_mean += sample.entropy;
        }
        entropy_mean /= history.GetCount();
        
        // Calculate variance
        for (const auto& sample : history) {
            entropy_variance += (sample.entropy - entropy_mean) * (sample.entropy - entropy_mean);
        }
        entropy_variance /= history.GetCount();
        double entropy_std_dev = sqrt(entropy_variance);
        
        scenario.severity = min(1.0, entropy_std_dev * 3.0);
        scenario.probability = min(1.0, entropy_std_dev * 2.5);
    }
    else if (type == "api_break") {
        // Severity based on entropy trend (if entropy is increasing rapidly)
        if (history.GetCount() >= 2) {
            double entropy_trend = 
                (history.Top().entropy - history[0].entropy) / 
                (history.Top().t - history[0].t);
            scenario.severity = max(0.0, min(1.0, entropy_trend * 10.0));
            scenario.probability = max(0.0, min(1.0, entropy_trend * 8.0));
        }
    }
    else if (type == "mass_refactor") {
        // Severity based on accumulated complexity
        double max_entropy = 0.0;
        for (const auto& sample : history) {
            if (sample.entropy > max_entropy) {
                max_entropy = sample.entropy;
            }
        }
        
        scenario.severity = min(1.0, max_entropy * 0.3);
        scenario.probability = min(1.0, max_entropy * 0.25);
    }
    else {
        // For unknown shock types, use default calculation based on entropy
        double avg_entropy = 0.0;
        for (const auto& sample : history) {
            avg_entropy += sample.entropy;
        }
        avg_entropy /= history.GetCount();
        
        scenario.severity = min(1.0, avg_entropy * 0.4);
        scenario.probability = min(1.0, avg_entropy * 0.3);
    }
    
    return scenario;
}