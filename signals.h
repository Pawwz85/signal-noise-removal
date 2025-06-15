#pragma once

#include <vector>
#include <complex>


#include <math.h> // for M_PI
typedef std::complex<double> complex;

template <class TimeUnit> class Signal;


template <class TimeUnit>
struct oscilogram_point {
	double value;
	TimeUnit timestamp;
};

static double hamming_window(double x) {
	const double a0 = 0.53836;
	const double a1 = 1 - a0;
	return a0 + a1 * cos(2 * M_PI * x);
}

static double Hana_window(double x) {
	return 1 - cos(2 * M_PI * x);
}

template <class TimeUnit>
class Signal {
	std::vector<oscilogram_point<TimeUnit>> probes;
public:

	void add_point(double value, TimeUnit timestamp) {
		auto it = probes.begin();
		while (it != probes.end() && (*it).timestamp < timestamp)
			++it;
		probes.insert(it, { value, timestamp });

	};

	size_t size() const { return probes.size(); };

	auto begin() { return probes.begin(); };
	auto end() { return probes.end(); };

	auto cbegin() const { return probes.cbegin(); };
	auto cend() const { return probes.cend(); };

};

static Signal<time_t>* load_signal(size_t probe_count, time_t probing_period, double * value_array) {
	time_t now = probing_period;
	Signal<time_t>* result = new Signal<time_t>;
	for (size_t i = 0; i < probe_count; ++i) {
		result->add_point(value_array[i], now);
		now += probing_period;
	}

	return result;
}

namespace FourierTransform {

	const complex COMPLEX_I = complex(0, 1);

	template <typename T>
	void __inplace_fourier_shuffle(T* const& buff, const size_t size) {
		// shuffles given buffer content, so first half of the buffer contains even values, and second odd values
		// After that operation, one could call DFT on each of these halves inside recursive fDFT implementation;  
		static std::vector<T> temp_buffer;
		temp_buffer.reserve(size);

		for (size_t i = 0; i < size; ++i)
			temp_buffer.push_back(buff[i]);

		for (size_t i = 0; i < size / 2; ++i) {
			buff[i] = temp_buffer[2 * i];
			buff[i + size / 2] = temp_buffer[2 * i + 1];
		}

		temp_buffer.clear();
	}

	template<bool inverted>
	void __unchecked_Fourier_transform(complex* result_buffer, const size_t size) {

		if (size == 1) {
			// Base case: DO NOTHING, because formula asks as to multiply by 1 in this case
			return;
		}

		__inplace_fourier_shuffle(result_buffer, size);

		const size_t half_size = size / 2;

		__unchecked_Fourier_transform<inverted>(result_buffer, half_size);
		__unchecked_Fourier_transform<inverted>(result_buffer + half_size, half_size);

		complex even, odd, exponent;

		for (size_t i = 0; i < half_size; ++i) {
			even = result_buffer[i];
			odd = result_buffer[i + half_size];
			
			if constexpr (inverted) {
				exponent = exp(+2.f * (M_PI)*COMPLEX_I * double(i) / double(size));
			}
			else {
				exponent = exp(-2.f * (M_PI)*COMPLEX_I * double(i) / double(size));
			}

			result_buffer[i] = even + odd * exponent;
			result_buffer[i + half_size] = even - odd * exponent;
					
		}
	}

	template<bool inverted>
	std::vector<complex> DFT(std::vector<complex> data) {
		size_t size = data.size();

		// this code sets 'size' value to the closest power of 2
		if (size & (size - 1)) {
			size_t lzcnt = __builtin_clzll(size << 1ull);
			size = 1ull << (63 - lzcnt);
		}

		complex* buff = new complex[size];

		for (size_t i = 0; i < size; ++i) buff[i] = 0;
		for (size_t i = 0; i < data.size(); ++i) buff[i] = data[i];

		__unchecked_Fourier_transform<inverted>(buff, size);

		std::vector<complex> result(buff, buff + size);
		delete[] buff;
		return result;
	};

	static std::vector<complex> DFT(Signal<time_t> signal) {
		std::vector<complex> casted;

		for (auto it = signal.cbegin(); it != signal.cend(); ++it) {
			casted.push_back(it->value);
		}

		return DFT<false>(casted);
	}

	static Signal<time_t> signal_synthesis(std::vector<complex>& Fourier_transform, time_t delta_t) {
		Signal<time_t > result;

		std::vector<complex> DFT_result = DFT<true>(Fourier_transform);

		time_t now = delta_t;

		for (const complex& c : DFT_result) {
			result.add_point(c.real(), now);
			now += delta_t;
		}

		auto it = result.begin();
		for (size_t i = 0; i < result.size(); ++i, ++it)
			it->value /= result.size();

		return result;
	}


	static Signal<time_t> denoise_signal_using_moving_average(const Signal<time_t> signal, size_t window_size, time_t delta_t) {
		std::vector<double> averages;

		std::vector<oscilogram_point<time_t>> org(signal.cbegin(), signal.cend());


		size_t j;
		double sum;
		for (size_t i = 0; i < org.size(); i+= window_size) {
			sum = 0;

			for (j = 0; (j < window_size) && (i + j < org.size()); ++j)
				sum += org[i + j].value;
			sum /= double(j);
			averages.push_back(sum);
		}	


		Signal<time_t> result;

		time_t now = delta_t * window_size;

		for (size_t i = 0; i < averages.size(); ++i){
			result.add_point(averages[i], now);
			now += delta_t * window_size;
		}

		return result;
	}
	
	static Signal<time_t> denoise_using_sinc_function(const Signal<time_t> signal, size_t window_size, time_t delta_t) {
		std::vector<complex> frequency_domain = DFT(signal);
		std::vector<complex> convolution_result;
		std::vector<double> sinc;

		for (size_t i = 0; i < frequency_domain.size(); ++i) 
			sinc.push_back(i ? sin(M_PI* i)/(M_PI* i): 1);
		

		size_t j;
		complex sum;
		for (size_t i = 0; i < frequency_domain.size(); ++i ) {
			sum = 0;
			for (size_t j = 0; j < i && j < window_size; ++j) {
				sum += frequency_domain[i] * sinc[i - 1 - j];
			}
			convolution_result.push_back(sum);
		}
		
		return signal_synthesis(convolution_result, delta_t);
	}

	static Signal<time_t> denoise_signal_by_using_filters(const Signal<time_t> signal, time_t delta_t) {
		
		std::vector<complex> dft = DFT(signal);
		std::vector<complex> reduced_dft;

		size_t N = dft.size();
		for (size_t i = 0; i < N; i++)
			dft[i] *= hamming_window(double(i) / N);

		return signal_synthesis(dft, delta_t);
	}



	static Signal<time_t> apply_window_function(const Signal<time_t> & original, double(*window_function)(double)) {
		Signal<time_t> result;
		auto it = original.cbegin();
		for (size_t i = 0; i < original.size(); ++i, ++it) {
			result.add_point(it->value * window_function(double(i) / original.size()), it->timestamp);
		}
		return result;
	}
}
