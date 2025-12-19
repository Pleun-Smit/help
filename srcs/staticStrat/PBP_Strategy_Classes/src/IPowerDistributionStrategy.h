/**
 * @interface IPowerDistributionStrategy
 * @brief Interface for power distribution strategies.
 *
 * Provides an API for algorithms that decide how to split the total available
 * power among charging stations. Implementations should encapsulate a specific
 * strategy (e.g., static/even distribution, priority-based, dynamic throttling).
 *
 * The destructor is virtual and defaulted to ensure proper cleanup of derived classes.
 */
class IPowerDistributionStrategy {
public:
    virtual ~IPowerDistributionStrategy() = default;

    /**
     * @brief Calculate power to distribute according to the strategy.
     *
     * Implementations should use the provided inputs to determine how much
     * power (in the same units as `totalPower`) to allocate to a charging
     * station or as a per-station allocation depending on the strategy.
     *
     * @param totalPower Total available power to distribute.
     * @param availableStations Number of stations that are alive.
     * @param activestations Number of currently active/charging stations.
     * @param MaxPowerChargingStation Maximum allowed power per charging station.
     * @return Amount of power to assign according to the strategy.
     */
    virtual float distributePower(float totalPower, int availableStations, int activestations, float MaxPowerChargingStation) = 0;
};
